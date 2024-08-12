# Gaze Analysis Workflow

1. Ensure Unity recordings are available in directory `{DATA_ROOT}/unityRecordings`, with recordings from each group separated into subdirectories with name `/groupX`

2. Use the `gaze_analysis` app in the [Recording-And-Analaysis-Plugin](https://github.com/gary444/Recording-And-Analysis-Plugin) repo to extract relevant quantitative data based on the users head transforms (specifically, head rotation speed data and angular distance between forward vector of each participant and their colleagues). Usage: `.\gaze_analysis.exe {DATA_ROOT}/unityRecordings {DATA_ROOT}/gazeAnalysis`

3. Use the `GazeClassificationPostProcessing` jupyter notebook to read the data written in the previous step, and classify the gaze state throughout each trial at a fixed sample rate. 

4. Use the `GazeStateAnalysis` jupyter notebook to read the classified data streams, calculate metrics based on the streams, and write those out to a single csv file (`gaze_classification_analysis_results.csv)`.

5. Use another script to determine significant diffences between the conditions and participants?




Checking the data:

1. The recording can be checked by replaying selected recordings in the Unity application (in repo [aplause-mr-projection-screen-study-netcode](https://github.com/vrsys/aplause-mr-projection-screen-study-netcode)). Branch: `feature/verify_gaze_classification`.

2. Also in that application, the `GazeClassificationResultViz` script can be attached to the `RecordingSetup` game object to visualize the gaze states assigned in step 3 by the `GazeClassificationPostProcessing` script.