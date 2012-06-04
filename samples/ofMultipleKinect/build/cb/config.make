# add custom variables to this file

# OF_ROOT allows to move projects outside apps/* just set this variable to the
# absoulte path to the OF root folder

OF_ROOT = /home/cubic/sdk/of_preRelease_v007_linux64/


# USER_CFLAGS allows to pass custom flags to the compiler
# for example search paths like:
# USER_CFLAGS = -I src/objects

USER_CFLAGS = -I ../../../../include/   #include of the _2Realwrapper


# USER_LDFLAGS allows to pass custom flags to the linker
# for example libraries like:
# USER_LD_FLAGS = libs/libawesomelib.a

USER_LDFLAGS = ../../../../bin/Release/lib_2RealKinectWrapper.a /usr/lib/libnimCodecs.so /usr/lib/libnimMockNodes.so /usr/lib/libnimRecorder.so /usr/lib/libXnCore.so /usr/lib/libXnDDK.so /usr/lib/libXnDeviceFile.so /usr/lib/libXnDeviceSensorV2KM.so /usr/lib/libXnFormats.so /usr/lib/libXnVCNITE_1_5_2.so /usr/lib/libXnVFeatures_1_5_2.so /usr/lib/libXnVFeatures_1_5_2.so /usr/lib/libXnVHandGenerator_1_5_2.so /usr/lib/libXnVNite_1_5_2.so /usr/lib/libOpenNI.so /usr/lib/libOpenNI.so

# use this to add system libraries for example:
# USER_LIBS = -lpango

USER_LIBS = -lOpenNI -lboost_thread

# change this to add different compiler optimizations to your project

USER_COMPILER_OPTIMIZATION = -std=c++0x -march=native -mtune=native -Os


EXCLUDE_FROM_SOURCE="bin,.xcodeproj,obj,.git"
