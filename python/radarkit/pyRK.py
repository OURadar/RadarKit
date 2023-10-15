import time
import tqdm
import ctypes
# import builtins
import numpy as np

from ._radarkit_ctypes_ import *
# from . import radial_noise

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# In this module the open and close functions are overwritten, use
# __builtins__.open and __builtins__.close to do necessary development.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

c_mps = 299792458
alignment = int(RKMemoryAlignSize / 4)
Productdict = {'Z': RKBaseProductIndexZ,
               'V': RKBaseProductIndexV,
               'W': RKBaseProductIndexW,
               'D': RKBaseProductIndexD,
               'R': RKBaseProductIndexR,
               'P': RKBaseProductIndexP}


class pyRKuint32(ctypes.c_uint32):
    pass


class UserParams(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('directory', ctypes.c_char * RKMaximumFolderPathLength),
        ('filename', ctypes.c_char * RKNameLength),
        ('verbose', ctypes.c_int),
        ('output', ctypes.c_bool),
        ('compressedOutput', ctypes.c_bool),
        ('SNRThreshold', ctypes.c_float),
        ('SQIThreshold', ctypes.c_float)
    ]


class Workspace(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('configs', ctypes.POINTER(RKConfig)),
        ('pulses', RKBuffer),
        ('rays', RKBuffer),
        ('fftModule', ctypes.POINTER(RKFFTModule)),
        ('pulseMachine', ctypes.POINTER(RKPulseEngine)),
        ('momentMachine', ctypes.POINTER(RKMomentEngine)),
        ('sweepMachine', ctypes.POINTER(RKSweepEngine)),
        ('recorder', ctypes.POINTER(RKRawDataRecorder)),
        ('ringMachine', ctypes.POINTER(RKPulseRingFilterEngine)),
        ('userModule', RKUserModule),
        ('userModuleFree', ctypes.CFUNCTYPE(UNCHECKED(None), RKUserModule)),
        ('configIndex', pyRKuint32),
        ('pulseIndex', pyRKuint32),
        ('rayIndex', pyRKuint32)
    ]

def open(filename, opmode='r'):
    fid = fopen(filename, opmode)
    out = RKFileHeaderRead(fid).contents
    out.fid = fid
    origin = ftell(fid)
    fseek(fid, 0, SEEK_END)
    out.filesize = ftell(fid)
    fseek(fid, origin, SEEK_SET)
    out.desc.configBufferDepth = 3
    # if pulseBufferDepth larger than pulses in file the rkid.raw_iq could fully
    # save in python memory ortherwise only last {pulseBufferDepth} pulses IQ is in python space.
    # larger the buffer more load happen in python (more copy and fetch to numpy array conversion)
    out.desc.pulseBufferDepth = 30000
    out.desc.rayBufferDepth = 400
    out.desc.dataPath = b'data'
    out.PulseFileManager = ReadPulseFromrkfile
    return out

def set_waveform(self, wfsig):
    wf = RKWaveformInitWithCountAndDepth(1, wfsig.size)
    wf.contents.filterAnchors[0][0].origin = 0
    wf.contents.filterAnchors[0][0].length = wfsig.size
    wf.contents.fc = 0
    # wf.contents.fs = 3e8*dic['trig_subpulse_length_dec']/dic['trig_subpulse_length']
    wf.contents.fs = self.fs
    wf.contents.type = RKWaveformTypeIsComplex
    # radarkit.show_Flag(rkid.config.waveform.contents.type, radarkit.RKWaveformTypeDict)
    wf.contents.name = b'Horus'
    wf.contents.filterCounts[0] = 1
    wf.contents.filterAnchors[0][0].name = 0
    wf.contents.filterAnchors[0][0].origin = 0
    wf.contents.filterAnchors[0][0].length = wf.contents.depth
    wf.contents.filterAnchors[0][0].maxDataLength = RKMaximumGateCount
    wf.contents.filterAnchors[0][0].subCarrierFrequency = 0
    place_RKComplex_array(wf.contents.samples[0], wfsig)
    RKWaveformNormalizeNoiseGain(wf)
    self.config.waveform = wf
    self.config.waveformDecimate = RKWaveformCopy(wf)
    RKWaveformDecimate(self.config.waveformDecimate, self.desc.pulseToRayRatio)


def close(self):
    fclose(self.fid)

def init_workspace(self, verbose=0, cores=4):
    if hasattr(self, 'workspace'):
        return
    else:
        desc = self.desc
        self.desc.initFlags = RKInitFlagAllocConfigBuffer | RKInitFlagAllocRawIQBuffer | RKInitFlagAllocMomentBuffer
        self.desc.initFlags |= RKInitFlagVerbose | RKInitFlagVeryVerbose
        if (self.dataType == RKRawDataTypeFromTransceiver):
            self.desc.initFlags |= RKInitFlagStartPulseEngine | RKInitFlagStartRingFilterEngine
        workspace = Workspace()
        ctypes.memset(ctypes.byref(workspace), 0, ctypes.sizeof(Workspace))

        workspace.fftModule = RKFFTModuleInit(desc.pulseCapacity, verbose)

        desc.pulseBufferSize = RKPulseBufferAlloc(ctypes.byref(workspace.pulses), desc.pulseCapacity, desc.pulseBufferDepth)
        desc.rayBufferSize = RKRayBufferAlloc(ctypes.byref(workspace.rays), desc.pulseCapacity // desc.pulseToRayRatio, desc.rayBufferDepth)
        desc.configBufferSize = desc.configBufferDepth * ctypes.sizeof(RKConfig)

        workspace.configs = ctypes.cast(malloc(desc.configBufferSize), ctypes.POINTER(RKConfig))

        workspace.pulseMachine = RKPulseEngineInit()
        RKPulseEngineSetVerbose(workspace.pulseMachine, verbose)
        RKPulseEngineSetFFTModule(workspace.pulseMachine, workspace.fftModule)

        RKPulseEngineSetEssentials(workspace.pulseMachine, ctypes.byref(desc),
                                           workspace.configs, ctypes.byref(workspace.configIndex),
                                           workspace.pulses, ctypes.byref(workspace.pulseIndex))
        RKPulseEngineSetCoreCount(workspace.pulseMachine, cores)
        if (desc.initFlags & RKInitFlagStartPulseEngine):
            RKPulseEngineStart(workspace.pulseMachine)

        workspace.ringMachine = RKPulseRingFilterEngineInit()
        RKPulseRingFilterEngineSetVerbose(workspace.ringMachine, verbose)
        RKPulseRingFilterEngineSetEssentials(workspace.ringMachine, ctypes.byref(desc),
                                                     workspace.configs, ctypes.byref(workspace.configIndex),
                                                     workspace.pulses, ctypes.byref(workspace.pulseIndex))
        RKPulseRingFilterEngineSetCoreCount(workspace.ringMachine, 2)
        if (desc.initFlags & RKInitFlagStartRingFilterEngine):
            RKPulseRingFilterEngineStart(workspace.ringMachine)

        workspace.momentMachine = RKMomentEngineInit()
        # RKMomentEngineSetNoiseEstimator(workspace.momentMachine, ctypes.POINTER(RKRayNoiseEstimator))
        workspace.momentMachine.contents.momentProcessor = ctypes.cast(RKMultiLag, type(workspace.momentMachine.contents.momentProcessor))
        workspace.momentMachine.contents.userLagChoice = 3
        workspace.momentMachine.contents.noiseEstimator = ctypes.cast(RKRayNoiseEstimator, type(workspace.momentMachine.contents.noiseEstimator))
        RKMomentEngineSetVerbose(workspace.momentMachine, verbose)
        RKMomentEngineSetFFTModule(workspace.momentMachine, workspace.fftModule)
        RKMomentEngineSetEssentials(workspace.momentMachine, ctypes.byref(desc),
                                            workspace.configs, ctypes.byref(workspace.configIndex),
                                            workspace.pulses, ctypes.byref(workspace.pulseIndex),
                                            workspace.rays, ctypes.byref(workspace.rayIndex))
        RKMomentEngineSetCoreCount(workspace.momentMachine, 2)
        RKMomentEngineStart(workspace.momentMachine)


        workspace.sweepMachine = RKSweepEngineInit()
        RKSweepEngineSetVerbose(workspace.sweepMachine, verbose)
        RKSweepEngineSetEssentials(workspace.sweepMachine, ctypes.byref(desc), None,
                                          workspace.configs, ctypes.byref(workspace.configIndex),
                                          workspace.rays, ctypes.byref(workspace.rayIndex))
        RKSweepEngineSetRecord(workspace.sweepMachine, True)
        RKSweepEngineStart(workspace.sweepMachine)

        workspace.recorder = RKRawDataRecorderInit()
        RKRawDataRecorderSetEssentials(workspace.recorder, ctypes.byref(desc), None,
                                               workspace.configs, ctypes.byref(workspace.configIndex),
                                               workspace.pulses, ctypes.byref(workspace.pulseIndex))
        RKRawDataRecorderSetRawDataType(workspace.recorder, RKRawDataTypeAfterMatchedFilter)
        RKRawDataRecorderSetVerbose(workspace.recorder, verbose)

        config = workspace.configs
        ctypes.memmove(config, ctypes.byref(self.config), ctypes.sizeof(RKConfig))
        config.contents.waveform = self.config.waveform
        config.contents.waveformDecimate = self.config.waveformDecimate
        self.workspace = workspace

def free_workspace(self, verbose=1):
    if not (hasattr(self, 'workspace')):
        print("Warning RKEngingError(workspace not set yet.)")
        return
    else:
        workspace = self.workspace
        if hasattr(self, 'fid'):
            close(self)
        RKPulseEngineWaitWhileBusy(workspace.pulseMachine)
        RKMomentEngineWaitWhileBusy(workspace.momentMachine)
        RKSweepEngineFlush(workspace.sweepMachine)
        RKRawDataRecorderSetRecord(workspace.recorder, False)
        RKReadPulseFromFileReference(None, None, None)

        if (workspace.userModule is not None) and (workspace.userModuleFree is not None):
            workspace.userModuleFree(workspace.userModule)
            if verbose:
                print('free userModule')

        RKPulseEngineStop(workspace.pulseMachine)
        RKPulseRingFilterEngineStop(workspace.ringMachine)
        RKMomentEngineStop(workspace.momentMachine)
        RKSweepEngineStop(workspace.sweepMachine)
        RKRawDataRecorderStop(workspace.recorder)

        RKPulseEngineFree(workspace.pulseMachine)
        RKPulseRingFilterEngineFree(workspace.ringMachine)
        RKMomentEngineFree(workspace.momentMachine)
        RKSweepEngineFree(workspace.sweepMachine)
        RKRawDataRecorderFree(workspace.recorder)

        RKFFTModuleFree(workspace.fftModule)
        free(workspace.configs)
        free(workspace.pulses)
        free(workspace.rays)
        RKFileHeaderFree(ctypes.byref(self))


class RKEngineError(Exception):
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)

    def __str__(self):
        return f"RKEngineError: {self.message}"

def compress(self):
    if not (hasattr(self, 'workspace')):
        raise RKEngineError("workspace not set yet.")
    else:
        workspace = self.workspace
        config = workspace.configs
        RKPulseEngineSetFilterByWaveform(workspace.pulseMachine, config.contents.waveform)
        RKRawDataRecorderSetRecord(workspace.recorder, False)
        self.PulseFileManager(self)

def ReadPulseFromrkfile(self):
    workspace = self.workspace
    config = workspace.configs
    if hasattr(self, 'fid'):
        # Probably add a progress bar in the future
        p1 = ftell(self.fid)
        with tqdm.tqdm(total=self.filesize, ncols=100, bar_format='{l_bar}{bar}|{elapsed}<{remaining}') as pbar:
            for ip in range(RKRawDataRecorderDefaultMaximumRecorderDepth):
                pulse = RKPulseEngineGetVacantPulse(workspace.pulseMachine, RKPulseStatusCompressed)
                z = 1
                while (workspace.pulseMachine.contents.maxWorkerLag > 0.8):
                    time.sleep(1e-4)
                    if (z % 1000 == 0):
                        s = z * 0.001;
                        RKLog(f'Waiting for workers ...  z = {z:d} / {s:.1f}s   {workspace.pulseMachine.contents.maxWorkerLag:.1f} \n')
                    z = z + 1
                r = RKReadPulseFromFileReference(pulse, ctypes.byref(self), self.fid)
                if (r != RKResultSuccess):
                    self.npulses = ip
                    break
                # pulse.contents.header.i = ip
                pulse.contents.header.configIndex = 0
                p0 = ftell(self.fid)
                pbar.update(p0 - p1)
                p1 = p0
            pbar.close()
    else:
        raise RKEngineError("Unknown pulses for compression.")

def get_iq(self, iqtype='c'):
    if not (hasattr(self, 'workspace')):
        raise RKEngineError("workspace not set yet.")
    else:
        workspace = self.workspace
        RKPulseEngineWaitWhileBusy(workspace.pulseMachine)
        pulseCount = np.min([self.npulses, self.desc.pulseBufferDepth])
        # Get the first pulse to retrieve gateCount / downSampledGateCount
        pulse = RKGetPulseFromBuffer(workspace.pulses, 0)
        gateCount = pulse.contents.header.gateCount if iqtype == 'r' else pulse.contents.header.downSampledGateCount;
        iq = np.zeros((pulseCount, 2, gateCount), dtype=np.complex64)
        read_fn = read_raw_data if iqtype == 'r' else read_compressed_data
        for ip in tqdm.trange(pulseCount, ncols=100, bar_format='{l_bar}{bar}|{elapsed}<{remaining}'):
            pulse = RKGetPulseFromBuffer(workspace.pulses, ip)
#            show_Flag(pulse.contents.header.s, RKPulseStatusDict)
            for ic in range(2):
                iq[ip, ic, :] = read_fn(pulse, ic, gateCount)
        return iq

def set_iq(self, array, iqtype='c'):
    if not (hasattr(self, 'workspace')):
        raise RKEngineError("workspace not set yet.")
    else:
        workspace = self.workspace


def get_moment(self, variable_list=['Z', 'V', 'W', 'D', 'R', 'P']):
    if not (hasattr(self, 'workspace')):
        raise RKEngineError("workspace not set yet.")
    else:
        workspace = self.workspace
        RKMomentEngineWaitWhileBusy(workspace.momentMachine)
        self.variables = {}
        for varname in variable_list:
            buf = []
            for ip in range(360):
                ray = RKGetRayFromBuffer(workspace.sweepMachine.contents.rayBuffer, ip)
                data = read_RKFloat_array(RKGetFloatDataFromRay(ray, Productdict[varname]), ray.contents.header.gateCount)
                if data is None or data.size == 0:
                    break
                else:
                    buf.append(data)
            self.variables.update({varname: np.asarray(buf)})
        return self.variables


def read_RKFloat_array(rkpos, count):
    return np.ctypeslib.as_array(ctypes.cast(rkpos, ctypes.POINTER(RKFloat)), (count,))


def read_RKInt16C_array(rkpos, count):
    bufiq = np.ctypeslib.as_array(ctypes.cast(rkpos, ctypes.POINTER(ctypes.c_int16)), (count, 2))
    return bufiq[:, 0] + 1j * bufiq[:, 1]


def read_RKComplex_array(rkpos, count):
    bufiq = np.ctypeslib.as_array(ctypes.cast(rkpos, ctypes.POINTER(ctypes.c_float)), (count, 2))
    return bufiq[:, 0] + 1j * bufiq[:, 1]


def read_raw_data(pulse, c, count):
    return read_RKInt16C_array(RKGetInt16CDataFromPulse(pulse, c), count)


def read_compressed_data(pulse, c, count):
    return read_RKComplex_array(RKGetComplexDataFromPulse(pulse, c), count)


def place_RKComplex_array(dest, source):
    bufiq = np.zeros((source.size * 2), dtype=np.float32)
    bufiq[::2] = source.real
    bufiq[1::2] = source.imag
    ctypes.memmove(ctypes.cast(dest, ctypes.POINTER(ctypes.c_float)), bufiq.ctypes.data, bufiq.nbytes)


def place_RKInt16C_array(dest, source):
    bufiq = np.zeros((source.size * 2), dtype=np.int16)
    bufiq[::2] = source.real.astype(np.int16)
    bufiq[1::2] = source.imag.astype(np.int16)
    ctypes.memmove(ctypes.cast(dest, ctypes.POINTER(ctypes.c_int16)), bufiq.ctypes.data, bufiq.nbytes)


RKPulseStatusDict = {'RKPulseStatusNull': RKPulseStatusNull,
                     'RKPulseStatusVacant': RKPulseStatusVacant,
                     'RKPulseStatusHasIQData': RKPulseStatusHasIQData,
                     'RKPulseStatusHasPosition': RKPulseStatusHasPosition,
                     'RKPulseStatusInspected': RKPulseStatusInspected,
                     'RKPulseStatusCompressed': RKPulseStatusCompressed,
                     'RKPulseStatusSkipped': RKPulseStatusSkipped,
                     'RKPulseStatusDownSampled': RKPulseStatusDownSampled,
                     'RKPulseStatusProcessed': RKPulseStatusProcessed,
                     'RKPulseStatusRingInspected': RKPulseStatusRingInspected,
                     'RKPulseStatusRingFiltered': RKPulseStatusRingFiltered,
                     'RKPulseStatusRingSkipped': RKPulseStatusRingSkipped,
                     'RKPulseStatusRingProcessed': RKPulseStatusRingProcessed,
                     'RKPulseStatusReadyForMoments': RKPulseStatusReadyForMoments,
                     'RKPulseStatusUsedForMoments': RKPulseStatusUsedForMoments,
                     'RKPulseStatusProcessMask': RKPulseStatusProcessMask,
                     'RKPulseStatusRecorded': RKPulseStatusRecorded,
                     'RKPulseStatusStreamed': RKPulseStatusStreamed}

RKWaveformTypeDict = {'RKWaveformTypeNone': RKWaveformTypeNone,
                      'RKWaveformTypeIsComplex': RKWaveformTypeIsComplex,
                      'RKWaveformTypeSingleTone': RKWaveformTypeSingleTone,
                      'RKWaveformTypeFrequencyHopping': RKWaveformTypeFrequencyHopping,
                      'RKWaveformTypeLinearFrequencyModulation': RKWaveformTypeLinearFrequencyModulation,
                      'RKWaveformTypeTimeFrequencyMultiplexing': RKWaveformTypeTimeFrequencyMultiplexing,
                      'RKWaveformTypeFromFile': RKWaveformTypeFromFile,
                      'RKWaveformTypeFlatAnchors': RKWaveformTypeFlatAnchors,
                      'RKWaveformTypeFrequencyHoppingChirp': RKWaveformTypeFrequencyHoppingChirp}

RKMarkerDict = {'RKMarkerNull': RKMarkerNull,
                'RKMarkerSweepMiddle': RKMarkerSweepMiddle,
                'RKMarkerSweepBegin': RKMarkerSweepBegin,
                'RKMarkerSweepEnd': RKMarkerSweepEnd,
                'RKMarkerVolumeBegin': RKMarkerVolumeBegin,
                'RKMarkerVolumeEnd': RKMarkerVolumeEnd,
                'RKMarkerScanTypeMask': RKMarkerScanTypeMask,
                'RKMarkerScanTypeUnknown': RKMarkerScanTypeUnknown,
                'RKMarkerScanTypePPI': RKMarkerScanTypePPI,
                'RKMarkerScanTypeRHI': RKMarkerScanTypeRHI,
                'RKMarkerScanTytpePoint': RKMarkerScanTytpePoint,
                'RKMarkerMemoryManagement': RKMarkerMemoryManagement}


def show_Flag(RKstatus, sdict=RKPulseStatusDict):
    s = '-'
    for iks in sdict.keys():
        if RKstatus & sdict[iks]:
            s = s + ' | ' + iks
    print(s)

union_rk_file_header.compress = compress
union_rk_file_header.get_iq = get_iq
union_rk_file_header.get_moment = get_moment
union_rk_file_header.close = close
union_rk_file_header.init_workspace = init_workspace
union_rk_file_header.free_workspace = free_workspace
union_rk_file_header.set_waveform = set_waveform
