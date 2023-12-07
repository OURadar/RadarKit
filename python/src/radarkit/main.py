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

workspaces = []
alignment = int(RKMemoryAlignSize / 4)
Productdict = {
    "S": RKBaseProductIndexSh,
    "Z": RKBaseProductIndexZ,
    "V": RKBaseProductIndexV,
    "W": RKBaseProductIndexW,
    "D": RKBaseProductIndexD,
    "P": RKBaseProductIndexP,
    "R": RKBaseProductIndexR,
}


class pyRKuint32(ctypes.c_uint32):
    pass


class UserParams(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("directory", ctypes.c_char * RKMaximumFolderPathLength),
        ("filename", ctypes.c_char * RKNameLength),
        ("verbose", ctypes.c_int),
        ("output", ctypes.c_bool),
        ("compressedOutput", ctypes.c_bool),
        ("SNRThreshold", ctypes.c_float),
        ("SQIThreshold", ctypes.c_float),
    ]


class Workspace(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("configs", ctypes.POINTER(RKConfig)),
        ("pulses", RKBuffer),
        ("rays", RKBuffer),
        ("fftModule", ctypes.POINTER(RKFFTModule)),
        ("pulseMachine", ctypes.POINTER(RKPulseEngine)),
        ("momentMachine", ctypes.POINTER(RKMomentEngine)),
        ("sweepMachine", ctypes.POINTER(RKSweepEngine)),
        ("recorder", ctypes.POINTER(RKRawDataRecorder)),
        ("ringMachine", ctypes.POINTER(RKPulseRingFilterEngine)),
        ("userModule", RKUserModule),
        ("userModuleFree", ctypes.CFUNCTYPE(UNCHECKED(None), RKUserModule)),
        ("configIndex", pyRKuint32),
        ("pulseIndex", pyRKuint32),
        ("rayIndex", pyRKuint32),
    ]

    def __init__(self):
        super().__init__()
        self.name = f"\033{RKPythonColor[4:]}<  Python Core  >\033[m"
        self.verbose = 0
        self.fid = None
        self.desc = None
        self.header = None
        self.filesize = 0
        self.allocated = False
        RKSetProgramName(b"RadarKit")
        RKLog(f"{self.name} Initializing ...")

    def open(self, filename, cores=4):
        self.fid = RKFileOpen(filename, "r")
        self.header = RKFileHeaderRead(self.fid).contents
        self.filesize = RKFileGetSize(self.fid)

        if self.verbose:
            RKLog(
                f"{self.name} dataType = {self.header.dataType} out of [{RKRawDataTypeNull} {RKRawDataTypeFromTransceiver} {RKRawDataTypeAfterMatchedFilter}]"
            )

        # Store as self.desc using the first header.desc and override some attributes. Only the first encounter matters.
        if self.desc is None or self.allocated is False:
            desc = self.header.desc
            desc.dataPath = b"data"
            desc.configBufferDepth = 3
            desc.pulseBufferDepth = RKMaximumPulsesPerRay + 50
            desc.rayBufferDepth = RKMaximumRaysPerSweep + 50
            desc.initFlags = (
                RKInitFlagAllocConfigBuffer | RKInitFlagAllocRawIQBuffer | RKInitFlagAllocMomentBuffer | RKInitFlagVerbose
            )
            if self.header.dataType == RKRawDataTypeFromTransceiver:
                desc.initFlags |= RKInitFlagStartPulseEngine | RKInitFlagStartRingFilterEngine
            self.configIndex = pyRKuint32(desc.configBufferDepth - 1)
            self.desc = desc
            self.alloc(cores=cores)

        if self.desc.pulseCapacity != self.header.desc.pulseCapacity:
            raise RKEngineError("pulseCapacity mismatch. Please use a new workspace.")
        if ((self.desc.initFlags & RKInitFlagStartPulseEngine) and (self.header.dataType == RKRawDataTypeAfterMatchedFilter)) or (
            not (self.desc.initFlags & RKInitFlagStartPulseEngine) and (self.header.dataType == RKRawDataTypeFromTransceiver)
        ):
            raise RKEngineError("dataType mismatch. Please use a new workspace.")

        k = next_modulo_s(self.configIndex.value, self.desc.configBufferDepth)
        config = self.configs[k]
        ctypes.memmove(ctypes.byref(config), ctypes.byref(self.header.config), ctypes.sizeof(RKConfig))
        config.waveform = self.header.config.waveform
        config.waveformDecimate = self.header.config.waveformDecimate
        self.configIndex.value = k

    def close(self):
        if self.fid is None:
            return
        RKFileClose(self.fid)
        self.fid = None

    def alloc(self, verbose=0, cores=4):
        desc = self.desc

        self.fftModule = RKFFTModuleInit(desc.pulseCapacity, verbose)

        desc.configBufferSize = RKConfigBufferAlloc(ctypes.byref(self.configs), desc.configBufferDepth)
        desc.pulseBufferSize = RKPulseBufferAlloc(ctypes.byref(self.pulses), desc.pulseCapacity, desc.pulseBufferDepth)
        desc.rayBufferSize = RKRayBufferAlloc(
            ctypes.byref(self.rays), desc.pulseCapacity // desc.pulseToRayRatio, desc.rayBufferDepth
        )

        configIndexPtr = ctypes.byref(self.configIndex)
        pulseIndexPtr = ctypes.byref(self.pulseIndex)
        rayIndexPtr = ctypes.byref(self.rayIndex)
        descPtr = ctypes.byref(desc)

        self.pulseMachine = RKPulseEngineInit()
        RKPulseEngineSetVerbose(self.pulseMachine, verbose)
        RKPulseEngineSetEssentials(
            self.pulseMachine, descPtr, self.fftModule, self.configs, configIndexPtr, self.pulses, pulseIndexPtr
        )
        RKPulseEngineSetCoreCount(self.pulseMachine, cores)
        if desc.initFlags & RKInitFlagStartPulseEngine:
            RKPulseEngineStart(self.pulseMachine)

        self.ringMachine = RKPulseRingFilterEngineInit()
        RKPulseRingFilterEngineSetVerbose(self.ringMachine, verbose)
        RKPulseRingFilterEngineSetEssentials(self.ringMachine, descPtr, self.configs, configIndexPtr, self.pulses, pulseIndexPtr)
        RKPulseRingFilterEngineSetCoreCount(self.ringMachine, 2)
        if desc.initFlags & RKInitFlagStartRingFilterEngine:
            RKPulseRingFilterEngineStart(self.ringMachine)
            RKPulseEngineSetWaitForRingFilter(self.pulseMachine, True)

        self.momentMachine = RKMomentEngineInit()
        RKMomentEngineSetVerbose(self.momentMachine, verbose)
        RKMomentEngineSetEssentials(
            self.momentMachine,
            descPtr,
            self.fftModule,
            self.configs,
            configIndexPtr,
            self.pulses,
            pulseIndexPtr,
            self.rays,
            rayIndexPtr,
        )
        RKMomentEngineSetCoreCount(self.momentMachine, 2)
        RKMomentEngineSetMomentProcessorToMultiLag3(self.momentMachine)
        RKMomentEngineStart(self.momentMachine)

        self.sweepMachine = RKSweepEngineInit()
        RKSweepEngineSetVerbose(self.sweepMachine, verbose)
        RKSweepEngineSetEssentials(self.sweepMachine, descPtr, None, self.configs, configIndexPtr, self.rays, rayIndexPtr)
        RKSweepEngineSetRecord(self.sweepMachine, True)
        RKSweepEngineStart(self.sweepMachine)

        self.recorder = RKRawDataRecorderInit()
        RKRawDataRecorderSetEssentials(self.recorder, descPtr, None, self.configs, configIndexPtr, self.pulses, pulseIndexPtr)
        RKRawDataRecorderSetRawDataType(self.recorder, RKRawDataTypeAfterMatchedFilter)
        RKRawDataRecorderSetVerbose(self.recorder, verbose)

        self.allocated = True

        global workspaces
        workspaces.append(self)

    def free(self):
        if self.fid:
            self.close()
        scratch = self.sweepMachine.contents.scratchSpaces[self.sweepMachine.contents.scratchSpaceIndex]
        count = scratch.rayCount + self.sweepMachine.contents.business
        if self.verbose:
            print(f"count = {count}")
        if count > 2:
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

        self.allocated = False

        global workspaces
        workspaces.remove(self)

    def set_waveform(self, samples):
        waveform = RKWaveformInitWithCountAndDepth(1, samples.size)
        waveform.contents.filterAnchors[0][0].origin = 0
        waveform.contents.filterAnchors[0][0].length = samples.size
        waveform.contents.fc = 0
        waveform.contents.fs = self.fs
        waveform.contents.type = RKWaveformTypeIsComplex
        waveform.contents.name = b"Custom"
        waveform.contents.filterCounts[0] = 1
        waveform.contents.filterAnchors[0][0].name = 0
        waveform.contents.filterAnchors[0][0].origin = 0
        waveform.contents.filterAnchors[0][0].length = waveform.contents.depth
        waveform.contents.filterAnchors[0][0].maxDataLength = RKMaximumGateCount
        waveform.contents.filterAnchors[0][0].subCarrierFrequency = 0
        place_RKComplex_array(waveform.contents.samples[0], samples)
        RKWaveformNormalizeNoiseGain(waveform)

        config = self.configs[self.configIndex.value]
        config.waveform = waveform
        config.waveformDecimate = RKWaveformCopy(waveform)
        RKWaveformDecimate(config.waveformDecimate, self.desc.pulseToRayRatio)
        RKPulseEngineSetFilterByWaveform(self.pulseMachine, config.waveform)
        self.blindGateCount = samples.size

    def set_moment_method(self, method):
        if method not in ["ppp", "ppph", "multi2", "multi3", "multi4", "spec"]:
            raise RKEngineError(f"Invalid moment method: {method}")
        if method.startswith("multi"):
            self.momentMachine.contents.userLagChoice = int(method[-1])
            method = "multi"
        self.momentMachine.contents.momentProcessor = {
            "ppp": ctypes.cast(RKPulsePair, type(self.momentMachine.contents.momentProcessor)),
            "ppph": ctypes.cast(RKPulsePair, type(self.momentMachine.contents.momentProcessor)),
            "multi": ctypes.cast(RKMultiLag, type(self.momentMachine.contents.momentProcessor)),
            "spec": ctypes.cast(RKSpectralMoment, type(self.momentMachine.contents.momentProcessor)),
        }[method]

    def unset_user_module(self):
        if self.userModule is not None:
            self.userModuleFree(self.userModule)
        self.userModule = None
        self.userModuleFree = ctypes.cast(None, type(self.userModuleFree))
        RKPulseEngineUnsetCompressor(self.pulseMachine)
        RKMomentEngineUnsetCalibrator(self.momentMachine)

    def read(self, count=None):
        if self.fid is None:
            raise RKEngineError("No file is open.")
        if not self.allocated:
            raise RKEngineError("This workspace has been released.")

        RKRawDataRecorderSetRecord(self.recorder, False)

        config = self.configs[self.configIndex.value]
        RKPulseEngineSetFilterByWaveform(self.pulseMachine, config.waveform)

        pos = RKFileTell(self.fid)
        pulse = RKPulseEngineGetVacantPulse(self.pulseMachine, RKPulseStatusCompressed)
        r = RKReadPulseFromFileReference(pulse, ctypes.byref(self.header), self.fid)
        if r != RKResultSuccess:
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
        pulse.contents.header.s |= RKPulseStatusHasIQData | RKPulseStatusHasPosition
        while pulse.contents.header.s & RKPulseStatusProcessed == 0:
            time.sleep(0.01)
        downSampledGateCount = pulse.contents.header.downSampledGateCount
        print(
            f"Estimated number of pulses = {pulseCount:,d}    gateCount = {gateCount:,d}   downSampledGateCount = {downSampledGateCount:,d}"
        )
        if count is not None:
            pulseCount = min(count, pulseCount)
        riq = (
            np.zeros((pulseCount, 2, gateCount), dtype=np.complex64)
            if self.header.dataType == RKRawDataTypeFromTransceiver
            else None
        )
        ciq = np.zeros((pulseCount, 2, downSampledGateCount), dtype=np.complex64)
        az = np.zeros((pulseCount,), dtype=np.float32)
        el = np.zeros((pulseCount,), dtype=np.float32)
        pulseCount -= 1
        with tqdm.tqdm(total=pulseCount, ncols=100, bar_format="{l_bar}{bar}|{elapsed}<{remaining}") as pbar:
            ic = 0
            for ip in range(pulseCount):
                pulse = RKPulseEngineGetVacantPulse(self.pulseMachine, RKPulseStatusCompressed)

                z = 1
                while self.pulseMachine.contents.maxWorkerLag > 0.7:
                    time.sleep(0.01)
                    if z % 100 == 0:
                        s = z * 0.01
                        RKLog(f"Waiting for workers ...  z = {z:d} / {s:.1f}s   {self.pulseMachine.contents.maxWorkerLag:.1f} \n")
                    z = z + 1

                r = RKReadPulseFromFileReference(pulse, ctypes.byref(self.header), self.fid)
                if r != RKResultSuccess:
                    self.pulseIndex.value = previous_modulo_s(self.pulseIndex.value, self.desc.pulseBufferDepth)
                    print(f"No more data to read.")
                    break
                pulse.contents.header.configIndex = self.configIndex.value
                if self.header.dataType == RKRawDataTypeAfterMatchedFilter:
                    pulse.contents.header.s |= RKPulseStatusReadyForMoments
                pulse.contents.header.s |= RKPulseStatusHasIQData | RKPulseStatusHasPosition
                if self.header.dataType == RKRawDataTypeFromTransceiver:
                    riq[ip, :, :] = read_RKInt16C_from_pulse(pulse, gateCount)
                el[ip] = pulse.contents.header.elevationDegrees
                az[ip] = pulse.contents.header.azimuthDegrees

                pulse = self.get_done_pulse()
                while pulse != None:
                    ciq[ic, :, :] = read_RKComplex_from_pulse(pulse, downSampledGateCount)
                    ic += 1
                    pulse = self.get_done_pulse()

                pbar.update(1.0)
            pbar.close()

            if self.verbose:
                print(f"ip = {ip:,d}   ic = {ic:,d}   pulseCount = {pulseCount:,d}")
            s = 0
            while ic < ip and s < 30:
                pulse = self.get_done_pulse()
                if pulse is None:
                    time.sleep(0.1)
                    s += 1
                    continue
                ciq[ic, :, :] = read_RKComplex_from_pulse(pulse, downSampledGateCount)
                ic += 1
            if self.verbose or s >= 30 or ic < ip:
                print(f"ip = {ip:,d}   ic = {ic:,d}   pulseCount = {pulseCount:,d}")

        RKFileSeek(self.fid, pos)
        RKMomentEngineWaitWhileBusy(self.momentMachine)

        return {"riq": riq, "ciq": ciq, "el": el, "az": az}

    def get_current_config(self):
        return self.configs[self.configIndex.value]

    def get_done_pulse(self):
        pulse = RKPulseEngineGetProcessedPulse(self.pulseMachine, None)
        return pulse if pulse else None

    def get_moment(self, offset=None, variable_list=["Z", "V", "W", "D", "R", "P"]):
        if offset is None:
            k = self.sweepMachine.contents.scratchSpaceIndex
            RKSweepEngineFlush(self.sweepMachine)
        else:
            k = offset
        scratch = self.sweepMachine.contents.scratchSpaces[k]
        if self.verbose:
            print(f"Gathering {scratch.rayCount} rays from scratch space #{k} ...")
        self.variables = {}
        gateCount = scratch.rays[0].contents.header.gateCount
        scanTime = scratch.rays[0].contents.header.startTime.tv_sec
        for varname in variable_list:
            buf = []
            for k in range(scratch.rayCount):
                ray = scratch.rays[k]
                data = read_RKFloat_array(RKGetFloatDataFromRay(ray, Productdict[varname]), gateCount)
                if data is None or data.size == 0:
                    print(f"Breaking away at k = {k} ...")
                    break
                else:
                    buf.append(data)
            self.variables.update({varname: np.asarray(buf)})
        self.variables.update({"time": scanTime})
        return self.variables


def open(filename, force=False):
    global workspaces
    if force or len(workspaces) == 0:
        workspace = Workspace()
    else:
        workspace = workspaces[-1]
    workspace.open(filename)
    if len(workspaces) > 3:
        print(f"Warning. Multiple workspaces ({len(workspaces)}) detected.")
    return workspace


def close():
    global workspaces
    for workspace in workspaces:
        workspace.free()
    workspaces = []


def last():
    global workspaces
    if len(workspaces) == 0:
        raise RKEngineError("No workspace is open.")
    return workspaces[-1]


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


def read_RKInt16C_from_pulse(pulse, count):
    h = ctypes.cast(RKGetInt16CDataFromPulse(pulse, 0), ctypes.POINTER(ctypes.c_int16))
    h = np.ctypeslib.as_array(h, (count * 2,)).astype(np.float32).view(np.complex64)
    v = ctypes.cast(RKGetInt16CDataFromPulse(pulse, 1), ctypes.POINTER(ctypes.c_int16))
    v = np.ctypeslib.as_array(v, (count * 2,)).astype(np.float32).view(np.complex64)
    x = np.vstack((h, v))
    x.imag = -x.imag
    return x


def read_RKComplex_from_pulse(pulse, count):
    h = ctypes.cast(RKGetComplexDataFromPulse(pulse, 0), ctypes.POINTER(ctypes.c_float))
    h = np.ctypeslib.as_array(h, (count * 2,)).view(np.complex64)
    v = ctypes.cast(RKGetComplexDataFromPulse(pulse, 1), ctypes.POINTER(ctypes.c_float))
    v = np.ctypeslib.as_array(v, (count * 2,)).view(np.complex64)
    x = np.vstack((h, v))
    x.imag = -x.imag
    return x


def place_RKComplex_array(dest, source):
    # np.conj(source, out=source)
    # ctypes.memmove(ctypes.cast(dest, ctypes.POINTER(ctypes.c_float)), source.ctypes.data, source.nbytes)
    bufiq = np.zeros((source.size * 2), dtype=np.float32)
    bufiq[::2] = source.real
    bufiq[1::2] = -source.imag
    ctypes.memmove(ctypes.cast(dest, ctypes.POINTER(ctypes.c_float)), bufiq.ctypes.data, bufiq.nbytes)


def place_RKInt16C_array(dest, source):
    bufiq = np.zeros((source.size * 2), dtype=np.int16)
    bufiq[::2] = source.real.astype(np.int16)
    bufiq[1::2] = -source.imag.astype(np.int16)
    ctypes.memmove(ctypes.cast(dest, ctypes.POINTER(ctypes.c_int16)), bufiq.ctypes.data, bufiq.nbytes)


RKPulseStatusDict = {
    "RKPulseStatusNull": RKPulseStatusNull,
    "RKPulseStatusVacant": RKPulseStatusVacant,
    "RKPulseStatusHasIQData": RKPulseStatusHasIQData,
    "RKPulseStatusHasPosition": RKPulseStatusHasPosition,
    "RKPulseStatusInspected": RKPulseStatusInspected,
    "RKPulseStatusCompressed": RKPulseStatusCompressed,
    "RKPulseStatusSkipped": RKPulseStatusSkipped,
    "RKPulseStatusDownSampled": RKPulseStatusDownSampled,
    "RKPulseStatusProcessed": RKPulseStatusProcessed,
    "RKPulseStatusRingInspected": RKPulseStatusRingInspected,
    "RKPulseStatusRingFiltered": RKPulseStatusRingFiltered,
    "RKPulseStatusRingSkipped": RKPulseStatusRingSkipped,
    "RKPulseStatusRingProcessed": RKPulseStatusRingProcessed,
    "RKPulseStatusReadyForMoments": RKPulseStatusReadyForMoments,
    "RKPulseStatusUsedForMoments": RKPulseStatusUsedForMoments,
    "RKPulseStatusProcessMask": RKPulseStatusProcessMask,
    "RKPulseStatusRecorded": RKPulseStatusRecorded,
    "RKPulseStatusStreamed": RKPulseStatusStreamed,
}

RKWaveformTypeDict = {
    "RKWaveformTypeNone": RKWaveformTypeNone,
    "RKWaveformTypeIsComplex": RKWaveformTypeIsComplex,
    "RKWaveformTypeSingleTone": RKWaveformTypeSingleTone,
    "RKWaveformTypeFrequencyHopping": RKWaveformTypeFrequencyHopping,
    "RKWaveformTypeLinearFrequencyModulation": RKWaveformTypeLinearFrequencyModulation,
    "RKWaveformTypeTimeFrequencyMultiplexing": RKWaveformTypeTimeFrequencyMultiplexing,
    "RKWaveformTypeFromFile": RKWaveformTypeFromFile,
    "RKWaveformTypeFlatAnchors": RKWaveformTypeFlatAnchors,
    "RKWaveformTypeFrequencyHoppingChirp": RKWaveformTypeFrequencyHoppingChirp,
}

RKMarkerDict = {
    "RKMarkerNull": RKMarkerNull,
    "RKMarkerSweepMiddle": RKMarkerSweepMiddle,
    "RKMarkerSweepBegin": RKMarkerSweepBegin,
    "RKMarkerSweepEnd": RKMarkerSweepEnd,
    "RKMarkerVolumeBegin": RKMarkerVolumeBegin,
    "RKMarkerVolumeEnd": RKMarkerVolumeEnd,
    "RKMarkerScanTypeMask": RKMarkerScanTypeMask,
    "RKMarkerScanTypeUnknown": RKMarkerScanTypeUnknown,
    "RKMarkerScanTypePPI": RKMarkerScanTypePPI,
    "RKMarkerScanTypeRHI": RKMarkerScanTypeRHI,
    "RKMarkerScanTytpePoint": RKMarkerScanTytpePoint,
    "RKMarkerMemoryManagement": RKMarkerMemoryManagement,
}


def show_flag(RKstatus, sdict=RKPulseStatusDict):
    s = "-"
    for iks in sdict.keys():
        if RKstatus & sdict[iks]:
            s = s + " | " + iks
    print(s)
