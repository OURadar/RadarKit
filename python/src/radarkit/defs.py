from ._ctypes_ import *

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
    "RKPulseStatusReadyForMomentEngine": RKPulseStatusReadyForMomentEngine,
    "RKPulseStatusCompleteForMoments": RKPulseStatusCompleteForMoments,
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
