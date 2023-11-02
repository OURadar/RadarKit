import time
import tqdm
import ctypes
# import builtins
import numpy as np

from ._ctypes_ import *

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
    fid = RKFileOpen(filename, opmode)
    out = RKFileHeaderRead(fid).contents
    # Add and override some attributes
    out.fid = fid
    out.filesize = RKFileGetSize(fid)
    out.desc.dataPath = b'data'
    out.desc.configBufferDepth = 10
    out.desc.pulseBufferDepth = RKRawDataRecorderDefaultMaximumRecorderDepth    # Large enough to hang on to all pulses.
    out.desc.rayBufferDepth = RKMaximumRaysPerSweep + 50                        # Large enough to hang on to all rays.
    return out

def close(self):
    RKFileClose(self.fid)

class core(union_rk_file_header):
    def __init__(self):
        super().__init__()
        self.name = f'\033{RKPythonColor[4:]}Python Core\033[m'
        RKSetProgramName(b"RadarKit")
        RKLog(f"{self.name} Initializing ...")

    def preparefromrkfile(self, rkfile):
        ctypes.memmove(ctypes.byref(self.config), ctypes.byref(rkfile.config), ctypes.sizeof(rkfile.config))
        ctypes.memmove(ctypes.byref(self.desc), ctypes.byref(rkfile.desc), ctypes.sizeof(rkfile.desc))
        self.dataType = rkfile.dataType
        self.version = rkfile.version
        self.ingest = ingest_rk_file

    def init_workspace(self, verbose=0, cores=4):
        desc = self.desc
        self.desc.initFlags = RKInitFlagAllocConfigBuffer | RKInitFlagAllocRawIQBuffer | RKInitFlagAllocMomentBuffer
        self.desc.initFlags |= RKInitFlagVerbose | RKInitFlagVeryVerbose
        if (self.dataType == RKRawDataTypeFromTransceiver):
            self.desc.initFlags |= RKInitFlagStartPulseEngine | RKInitFlagStartRingFilterEngine
        if verbose:
            RKLog(f'{self.name} dataType = {self.dataType} out of [{RKRawDataTypeNull} {RKRawDataTypeFromTransceiver} {RKRawDataTypeAfterMatchedFilter}]')

        workspace = Workspace()
        ctypes.memset(ctypes.byref(workspace), 0, ctypes.sizeof(Workspace))

        workspace.fftModule = RKFFTModuleInit(desc.pulseCapacity, verbose)

        desc.configBufferSize = RKConfigBufferAlloc(ctypes.byref(workspace.configs), desc.configBufferDepth)
        desc.pulseBufferSize = RKPulseBufferAlloc(ctypes.byref(workspace.pulses), desc.pulseCapacity, desc.pulseBufferDepth)
        desc.rayBufferSize = RKRayBufferAlloc(ctypes.byref(workspace.rays), desc.pulseCapacity // desc.pulseToRayRatio, desc.rayBufferDepth)

        workspace.pulseMachine = RKPulseEngineInit()
        RKPulseEngineSetVerbose(workspace.pulseMachine, verbose)
        RKPulseEngineSetEssentials(workspace.pulseMachine, ctypes.byref(desc), workspace.fftModule,
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
        workspace.momentMachine.contents.momentProcessor = ctypes.cast(RKMultiLag, type(workspace.momentMachine.contents.momentProcessor))
        workspace.momentMachine.contents.userLagChoice = 3
        # RKMomentEngineSetNoiseEstimator(workspace.momentMachine, ctypes.POINTER(RKRayNoiseEstimator))
        # workspace.momentMachine.contents.noiseEstimator = ctypes.cast(RKRayNoiseEstimator, type(workspace.momentMachine.contents.noiseEstimator))
        RKMomentEngineSetVerbose(workspace.momentMachine, verbose)
        RKMomentEngineSetEssentials(workspace.momentMachine, ctypes.byref(desc), workspace.fftModule,
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
        self.iq = iqcache()

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

    def free_workspace(self, verbose=1):
        if not (hasattr(self, 'workspace')):
            print("Warning RKEngingError(workspace not set yet.)")
            return
        else:
            workspace = self.workspace
            # if hasattr(self, 'fid'):
            #     close(self)
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
            RKConfigBufferFree(workspace.configs)
            RKPulseBufferFree(workspace.pulses)
            RKRayBufferFree(workspace.rays)

    def compress(self, rkfile):
        if not (hasattr(self, 'workspace')):
            raise RKEngineError("workspace not set yet.")
        else:
            workspace = self.workspace
            config = workspace.configs
            RKPulseEngineSetFilterByWaveform(workspace.pulseMachine, config.contents.waveform)
            RKRawDataRecorderSetRecord(workspace.recorder, False)
            self.ingest(self, rkfile)

    def get_iq(self, kind='c'):
        if not (hasattr(self, 'workspace')):
            raise RKEngineError("workspace not set yet.")
        else:
            workspace = self.workspace
            RKPulseEngineWaitWhileBusy(workspace.pulseMachine)
            sweepI = workspace.sweepMachine.contents.sweepIndex
            if self.iq.S < self.iq.E:
                npulses = self.iq.E - self.iq.S + 1
            elif self.iq.S > self.iq.E:
                npulses = self.desc.pulseBufferDepth - self.iq.S + self.iq.E + 1
            pulseCount = np.min([npulses, self.desc.pulseBufferDepth])

            if (sweepI == self.iq.last_request) and (pulseCount == self.iq.cache.shape[0]) and self.iq.last_kind == kind:
                print(f'Sweep {sweepI} from cached data.')
                return self.iq.cache
            else:
                # Get the first pulse to retrieve gateCount / downSampledGateCount
                pulse = RKGetPulseFromBuffer(workspace.pulses, self.iq.S)
                gateCount = pulse.contents.header.gateCount if kind == 'r' else pulse.contents.header.downSampledGateCount;
                iq = np.zeros((pulseCount, 2, gateCount), dtype=np.complex64)
                el = np.zeros(pulseCount, dtype=np.float32)
                az = np.zeros(pulseCount, dtype=np.float32)
                read_fn = read_raw_data if kind == 'r' else read_compressed_data
                for ip in tqdm.trange(pulseCount, ncols=100, bar_format='{l_bar}{bar}|{elapsed}<{remaining}'):
                    ik = ip + self.iq.S
                    if ik >= self.desc.pulseBufferDepth:
                        ik -= self.desc.pulseBufferDepth
                    pulse = RKGetPulseFromBuffer(workspace.pulses, ik)
        #            show_Flag(pulse.contents.header.s, RKPulseStatusDict)
                    for ic in range(2):
                        iq[ip, ic, :] = read_fn(pulse, ic, gateCount)
                    el[ip] = pulse.contents.header.elevationDegrees
                    az[ip] = pulse.contents.header.azimuthDegrees
                self.iq.last_request = sweepI
                self.iq.last_kind = kind
                self.iq.cache = iq
                self.iq.el = el
                self.iq.az = az
                print(f'Sweep {sweepI} from pulse buffer {self.iq.S}~{self.iq.E}')
            return iq

    def set_iq(self, array, kind='c'):
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

class iqcache:
    def __init__(self):
        self.S = np.nan
        self.E = np.nan
        self.St = np.nan
        self.last_request = np.nan
        self.cache = None

class RKEngineError(Exception):
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)

    def __str__(self):
        return f"RKEngineError: {self.message}"

def ingest_rk_file(self, rkfile):
    workspace = self.workspace
    config = workspace.configs[workspace.pulseMachine.contents.configIndex.contents.value]
    ctypes.memmove(ctypes.byref(config), ctypes.byref(rkfile.config), ctypes.sizeof(RKConfig))
    RKPulseEngineSetFilterByWaveform(workspace.pulseMachine, config.waveform)
    if hasattr(rkfile, 'fid'):
        p1 = RKFileTell(rkfile.fid)
        with tqdm.tqdm(total=rkfile.filesize, ncols=100, bar_format='{l_bar}{bar}|{elapsed}<{remaining}') as pbar:
            ik = workspace.pulseMachine.contents.pulseIndex.contents.value
            self.iq.S = ik
            for ip in range(RKRawDataRecorderDefaultMaximumRecorderDepth):
                pulse = RKPulseEngineGetVacantPulse(workspace.pulseMachine, RKPulseStatusCompressed)
                # pulse = RKPulseEngineGetVacantPulse(workspace.pulseMachine, RKPulseStatusNull)
                # z = 1
                # while (workspace.pulseMachine.contents.maxWorkerLag > 0.8):
                #     time.sleep(1e-4)
                #     if (z % 1000 == 0):
                #         s = z * 0.001;
                #         RKLog(f'Waiting for workers ...  z = {z:d} / {s:.1f}s   {workspace.pulseMachine.contents.maxWorkerLag:.1f} \n')
                #     z = z + 1
                r = RKReadPulseFromFileReference(pulse, ctypes.byref(rkfile), rkfile.fid)
                if (r != RKResultSuccess):
                    self.npulses = ip
                    workspace.pulseMachine.contents.pulseIndex.contents.value = previous_modulo_s(workspace.pulseMachine.contents.pulseIndex.contents.value, self.desc.pulseBufferDepth)
                    break
                # pulse.contents.header.i = ip
                pulse.contents.header.configIndex = workspace.pulseMachine.contents.configIndex.contents.value
                pulse.contents.header.s = RKPulseStatusHasIQData | RKPulseStatusHasPosition
                if rkfile.dataType == RKRawDataTypeAfterMatchedFilter:
                    pulse.contents.header.s |= RKPulseStatusReadyForMoments
                p0 = RKFileTell(rkfile.fid)
                pbar.update(p0 - p1)
                p1 = p0
            pbar.close()
            ik = workspace.pulseMachine.contents.pulseIndex.contents.value
            self.iq.E = ik - 1
            workspace.pulseMachine.contents.configIndex.contents.value = next_modulo_s(workspace.pulseMachine.contents.configIndex.contents.value, self.desc.configBufferDepth)
    else:
        raise RKEngineError("Unknown pulses for compression.")

def previous_modulo_s(i, S):
    return (S - 1) if i == 0 else (i - 1)

def next_modulo_s(i, S):
    return 0 if i == S - 1 else i + 1

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

def show_flag(RKstatus, sdict=RKPulseStatusDict):
    s = '-'
    for iks in sdict.keys():
        if RKstatus & sdict[iks]:
            s = s + ' | ' + iks
    print(s)

union_rk_file_header.close = close
