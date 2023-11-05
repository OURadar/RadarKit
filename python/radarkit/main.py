import time
import tqdm
import ctypes

import numpy as np

from ._ctypes_ import *

# from . import radial_noise

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# In this module the open and close functions are overwritten, use
# __builtins__.open and __builtins__.close to do necessary development.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

alignment = int(RKMemoryAlignSize / 4)
Productdict = {'Z': RKBaseProductIndexZ,
               'V': RKBaseProductIndexV,
               'W': RKBaseProductIndexW,
               'D': RKBaseProductIndexD,
               'R': RKBaseProductIndexR,
               'P': RKBaseProductIndexP}
workspace = None

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

    def __init__(self):
        super().__init__()
        self.name = f'\033{RKPythonColor[4:]}<  Python Core  >\033[m'
        self.fid = None
        self.filesize = 0
        self.ingest = None
        self.verbose = 0
        RKSetProgramName(b"RadarKit")
        RKLog(f"{self.name} Initializing ...")
        # print(f'configIndex = {self.configIndex.value}')

    def populate(self, fid, header):
        self.fid = fid
        self.header = header
        self.filesize = RKFileGetSize(fid)

        RKLog(f'{self.name} dataType = {self.header.dataType} out of [{RKRawDataTypeNull} {RKRawDataTypeFromTransceiver} {RKRawDataTypeAfterMatchedFilter}]')

        if self.ingest is None:
            # Get original description from header and override some attributes
            desc = header.desc
            desc.dataPath = b'data'
            desc.configBufferDepth = 3
            desc.pulseBufferDepth = RKMaximumPulsesPerRay + 50
            desc.rayBufferDepth = RKMaximumRaysPerSweep + 50
            desc.initFlags = RKInitFlagAllocConfigBuffer | RKInitFlagAllocRawIQBuffer | RKInitFlagAllocMomentBuffer
            desc.initFlags |= RKInitFlagVerbose | RKInitFlagVeryVerbose
            if self.header.dataType == RKRawDataTypeFromTransceiver:
                desc.initFlags |= RKInitFlagStartPulseEngine | RKInitFlagStartRingFilterEngine
            self.desc = desc
            self.alloc()
            self.ingest = self.ingest_iq

        if self.desc.pulseCapacity != desc.pulseCapacity:
            raise RKEngineError("pulseCapacity mismatch. Please restart.")

        config = self.configs[self.configIndex.value]
        ctypes.memmove(ctypes.byref(config), ctypes.byref(self.header.config), ctypes.sizeof(RKConfig))
        config.waveform = self.header.config.waveform
        config.waveformDecimate = self.header.config.waveformDecimate

    def alloc(self, verbose=0, cores=4):
        desc = self.desc

        self.fftModule = RKFFTModuleInit(desc.pulseCapacity, verbose)

        desc.configBufferSize = RKConfigBufferAlloc(ctypes.byref(self.configs), desc.configBufferDepth)
        desc.pulseBufferSize = RKPulseBufferAlloc(ctypes.byref(self.pulses), desc.pulseCapacity, desc.pulseBufferDepth)
        desc.rayBufferSize = RKRayBufferAlloc(ctypes.byref(self.rays), desc.pulseCapacity // desc.pulseToRayRatio, desc.rayBufferDepth)

        self.pulseMachine = RKPulseEngineInit()
        RKPulseEngineSetVerbose(self.pulseMachine, verbose)
        RKPulseEngineSetEssentials(self.pulseMachine, ctypes.byref(desc), self.fftModule,
                                   self.configs, ctypes.byref(self.configIndex),
                                   self.pulses, ctypes.byref(self.pulseIndex))
        RKPulseEngineSetCoreCount(self.pulseMachine, cores)
        if (desc.initFlags & RKInitFlagStartPulseEngine):
            RKPulseEngineStart(self.pulseMachine)

        self.ringMachine = RKPulseRingFilterEngineInit()
        RKPulseRingFilterEngineSetVerbose(self.ringMachine, verbose)
        RKPulseRingFilterEngineSetEssentials(self.ringMachine, ctypes.byref(desc),
                                             self.configs, ctypes.byref(self.configIndex),
                                             self.pulses, ctypes.byref(self.pulseIndex))
        RKPulseRingFilterEngineSetCoreCount(self.ringMachine, 2)
        if (desc.initFlags & RKInitFlagStartRingFilterEngine):
            RKPulseRingFilterEngineStart(self.ringMachine)

        self.momentMachine = RKMomentEngineInit()
        self.momentMachine.contents.momentProcessor = ctypes.cast(RKMultiLag, type(self.momentMachine.contents.momentProcessor))
        self.momentMachine.contents.userLagChoice = 3
        RKMomentEngineSetVerbose(self.momentMachine, verbose)
        RKMomentEngineSetEssentials(self.momentMachine, ctypes.byref(desc), self.fftModule,
                                    self.configs, ctypes.byref(self.configIndex),
                                    self.pulses, ctypes.byref(self.pulseIndex),
                                    self.rays, ctypes.byref(self.rayIndex))
        RKMomentEngineSetCoreCount(self.momentMachine, 2)
        RKMomentEngineStart(self.momentMachine)

        self.sweepMachine = RKSweepEngineInit()
        RKSweepEngineSetVerbose(self.sweepMachine, verbose)
        RKSweepEngineSetEssentials(self.sweepMachine, ctypes.byref(desc), None,
                                   self.configs, ctypes.byref(self.configIndex),
                                   self.rays, ctypes.byref(self.rayIndex))
        RKSweepEngineSetRecord(self.sweepMachine, True)
        RKSweepEngineStart(self.sweepMachine)

        self.recorder = RKRawDataRecorderInit()
        RKRawDataRecorderSetEssentials(self.recorder, ctypes.byref(desc), None,
                                       self.configs, ctypes.byref(self.configIndex),
                                       self.pulses, ctypes.byref(self.pulseIndex))
        RKRawDataRecorderSetRawDataType(self.recorder, RKRawDataTypeAfterMatchedFilter)
        RKRawDataRecorderSetVerbose(self.recorder, verbose)

    def free(self):
        RKPulseEngineWaitWhileBusy(self.pulseMachine)
        RKMomentEngineWaitWhileBusy(self.momentMachine)
        RKSweepEngineFlush(self.sweepMachine)
        RKRawDataRecorderSetRecord(self.recorder, False)
        RKReadPulseFromFileReference(None, None, None)

        if (self.userModule is not None) and (self.userModuleFree is not None):
            self.userModuleFree(self.userModule)

        RKPulseEngineStop(self.pulseMachine)
        RKPulseRingFilterEngineStop(self.ringMachine)
        RKMomentEngineStop(self.momentMachine)
        RKSweepEngineStop(self.sweepMachine)
        RKRawDataRecorderStop(self.recorder)

        RKPulseEngineFree(self.pulseMachine)
        RKPulseRingFilterEngineFree(self.ringMachine)
        RKMomentEngineFree(self.momentMachine)
        RKSweepEngineFree(self.sweepMachine)
        RKRawDataRecorderFree(self.recorder)

        RKFFTModuleFree(self.fftModule)
        RKConfigBufferFree(self.configs)
        RKPulseBufferFree(self.pulses)
        RKRayBufferFree(self.rays)

    def set_waveform(self, samples):
        waveform = RKWaveformInitWithCountAndDepth(1, samples.size)
        waveform.contents.filterAnchors[0][0].origin = 0
        waveform.contents.filterAnchors[0][0].length = samples.size
        waveform.contents.fc = 0
        waveform.contents.fs = self.fs
        waveform.contents.type = RKWaveformTypeIsComplex
        waveform.contents.name = b'Custom'
        waveform.contents.filterCounts[0] = 1
        waveform.contents.filterAnchors[0][0].name = 0
        waveform.contents.filterAnchors[0][0].origin = 0
        waveform.contents.filterAnchors[0][0].length = waveform.contents.depth
        waveform.contents.filterAnchors[0][0].maxDataLength = RKMaximumGateCount
        waveform.contents.filterAnchors[0][0].subCarrierFrequency = 0
        place_RKComplex_array(waveform.contents.samples[0], samples)
        RKWaveformNormalizeNoiseGain(waveform)

        config = self.configs
        config.waveform = waveform
        config.waveformDecimate = RKWaveformCopy(waveform)
        RKWaveformDecimate(config.waveformDecimate, self.desc.pulseToRayRatio)

    def read(self):
        if self.fid is None:
            raise RKEngineError("No file is open.")
        config = workspace.configs[self.configIndex.value]
        RKPulseEngineSetFilterByWaveform(workspace.pulseMachine, config.waveform)
        RKRawDataRecorderSetRecord(workspace.recorder, False)
        return self.ingest()

    def ingest_iq(self):
        config = self.configs[self.configIndex.value]
        RKPulseEngineSetFilterByWaveform(self.pulseMachine, config.waveform)

        pos = RKFileTell(self.fid)
        pulse = RKPulseEngineGetVacantPulse(self.pulseMachine, RKPulseStatusCompressed)
        r = RKReadPulseFromFileReference(pulse, ctypes.byref(self.header), self.fid)
        if (r != RKResultSuccess):
            self.pulseIndex.value = previous_modulo_s(self.pulseIndex.value, self.desc.pulseBufferDepth)
            raise RKEngineError("Failed to read the first pulse.")
        pulse.contents.header.configIndex = self.configIndex.value
        # Estimate the number of pulses
        if self.header.dataType == RKRawDataTypeAfterMatchedFilter:
            pulse.contents.header.s |= RKPulseStatusReadyForMoments
            gateCount = pulse.contents.header.downSampledGateCount
            pulseCount = (self.filesize - pos) // (ctypes.sizeof(RKPulseHeader) + 2 * gateCount * ctypes.sizeof(RKComplex))
        else:
            gateCount = pulse.contents.header.gateCount
            pulseCount = (self.filesize - pos) // (ctypes.sizeof(RKPulseHeader) + 2 * gateCount * ctypes.sizeof(RKInt16C))
            riq = np.zeros((pulseCount, 2, gateCount), dtype=np.complex64)
        pulse.contents.header.s = RKPulseStatusHasIQData | RKPulseStatusHasPosition
        # downSampledGateCount = pulse.contents.header.downSampledGateCount
        downSampledGateCount = (gateCount - config.waveform.contents.depth) // self.desc.pulseToRayRatio
        print(f'Estimated number of pulses = {pulseCount:,d}    gateCount = {gateCount:,d}   downSampledGateCount = {downSampledGateCount:,d}')
        ciq = np.zeros((pulseCount, 2, downSampledGateCount), dtype=np.complex64)
        ic = 0
        pulseCount -= 1
        with tqdm.tqdm(total=pulseCount, ncols=100, bar_format='{l_bar}{bar}|{elapsed}<{remaining}') as pbar:
            for ip in range(pulseCount):
                pulse = RKPulseEngineGetVacantPulse(self.pulseMachine, RKPulseStatusCompressed)

                z = 1
                while (self.pulseMachine.contents.maxWorkerLag > 0.7):
                    time.sleep(0.01)
                    if (z % 100 == 0):
                        s = z * 0.01;
                        RKLog(f'Waiting for workers ...  z = {z:d} / {s:.1f}s   {self.pulseMachine.contents.maxWorkerLag:.1f} \n')
                    z = z + 1

                r = RKReadPulseFromFileReference(pulse, ctypes.byref(self.header), self.fid)
                if (r != RKResultSuccess):
                    self.configIndex.value = next_modulo_s(self.configIndex.valuee, self.desc.configBufferDepth)
                    self.pulseIndex.value = previous_modulo_s(self.pulseIndex.value, self.desc.pulseBufferDepth)
                    print(f'no more data to read.')
                    break
                pulse.contents.header.configIndex = self.configIndex.value
                if self.header.dataType == RKRawDataTypeAfterMatchedFilter:
                    pulse.contents.header.s |= RKPulseStatusReadyForMoments
                pulse.contents.header.s |= RKPulseStatusHasIQData | RKPulseStatusHasPosition
                if self.header.dataType == RKRawDataTypeFromTransceiver:
                    riq[ip, :, :] = read_raw_data(pulse, gateCount)

                pulse = get_done_pulse()
                while (pulse != None):
                    ciq[ic, :, :] = read_compressed_data(pulse, downSampledGateCount)
                    ic += 1
                    pulse = get_done_pulse()

                pbar.update(1.0)
            pbar.close()

            if self.verbose > 1:
                print(f'ip = {ip:,d}   ic = {ic:,d}   pulseCount = {pulseCount:,d}')
            s = 0
            while ic < ip and s < 50:
                pulse = get_done_pulse()
                if pulse is None:
                    time.sleep(0.1)
                    s += 1
                    continue
                ciq[ic, :, :] = read_compressed_data(pulse, downSampledGateCount)
                ic += 1
            if self.verbose > 1 or s >= 50 or ic < ip:
                print(f'ip = {ip:,d}   ic = {ic:,d}   pulseCount = {pulseCount:,d}')

        close()
        print('Done')

        return riq, ciq

    def get_moment(self, variable_list=['Z', 'V', 'W', 'D', 'R', 'P']):
        RKMomentEngineWaitWhileBusy(self.momentMachine)
        self.variables = {}
        for varname in variable_list:
            buf = []
            for ip in range(360):
                ray = RKGetRayFromBuffer(self.sweepMachine.contents.rayBuffer, ip)
                data = read_RKFloat_array(RKGetFloatDataFromRay(ray, Productdict[varname]), ray.contents.header.gateCount)
                if data is None or data.size == 0:
                    break
                else:
                    buf.append(data)
            self.variables.update({varname: np.asarray(buf)})
        return self.variables


def open(filename, opmode='r'):
    fid = RKFileOpen(filename, opmode)
    header = RKFileHeaderRead(fid).contents
    global workspace
    if workspace is None:
        workspace = Workspace()
    workspace.populate(fid, header)
    return workspace

def close():
    global workspace
    RKFileClose(workspace.fid)
    workspace.fid = None

def free():
    global workspace
    if workspace is None:
        return
    workspace.free()
    workspace = None

def get_done_pulse():
    global workspace
    pulse = RKPulseEngineGetProcessedPulse(workspace.pulseMachine, None)
    try:
        _ = pulse.contents.header.i
        return pulse
    except ValueError:
        return None


class RKEngineError(Exception):
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)

    def __str__(self):
        return f"RKEngineError: {self.message}"

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

def read_raw_data(pulse, count):
    # return read_RKInt16C_array(RKGetInt16CDataFromPulse(pulse, c), count)
    h = np.ctypeslib.as_array(ctypes.cast(RKGetInt16CDataFromPulse(pulse, 0), ctypes.POINTER(ctypes.c_int16)), (count * 2,)).astype(np.float32).view(np.complex64)
    v = np.ctypeslib.as_array(ctypes.cast(RKGetInt16CDataFromPulse(pulse, 1), ctypes.POINTER(ctypes.c_int16)), (count * 2,)).astype(np.float32).view(np.complex64)
    return np.vstack((h, v))

def read_compressed_data(pulse, count):
    # return read_RKComplex_array(RKGetComplexDataFromPulse(pulse, c), count)
    h = np.ctypeslib.as_array(ctypes.cast(RKGetComplexDataFromPulse(pulse, 0), ctypes.POINTER(ctypes.c_float)), (count * 2,)).view(np.complex64)
    v = np.ctypeslib.as_array(ctypes.cast(RKGetComplexDataFromPulse(pulse, 1), ctypes.POINTER(ctypes.c_float)), (count * 2,)).view(np.complex64)
    return np.vstack((h, v))

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
