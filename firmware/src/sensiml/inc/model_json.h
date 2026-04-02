#ifndef __MODEL_JSON_H__
#define __MODEL_JSON_H__

const char recognition_model_string_json[] = {"{\"NumModels\":1,\"ModelIndexes\":{\"0\":\"FINGER_NOISE_WATER_RANK_4\"},\"ModelDescriptions\":[{\"Name\":\"FINGER_NOISE_WATER_RANK_4\",\"ClassMaps\":{\"1\":\"finger\",\"2\":\"noise\",\"3\":\"water\",\"0\":\"Unknown\"},\"ModelType\":\"PME\",\"FeatureFunctions\":[\"Skewness\",\"AbsoluteAreaofHighFrequency\",\"Kurtosis\",\"InterquartileRange\",\"75thPercentile\",\"100thPercentile\",\"GlobalPeaktoPeakofLowFrequency\",\"GlobalMinMaxSum\",\"MFCC\",\"MFCC\",\"MFCC\",\"MFCC\",\"MFCC\",\"DominantFrequency\",\"PowerSpectrum\",\"PowerSpectrum\",\"PowerSpectrum\",\"PowerSpectrum\",\"PowerSpectrum\",\"PowerSpectrum\",\"PowerSpectrum\",\"PowerSpectrum\",\"PeakFrequencies\",\"PeakFrequencies\"]}]}"};

int32_t recognition_model_string_json_len = sizeof(recognition_model_string_json);

#endif /* __MODEL_JSON_H__ */
