
#ifndef LS_COMPILER_TWEAKS_H
#define LS_COMPILER_TWEAKS_H

#pragma GCC diagnostic ignored "-Wunused-parameter"
// shut up warning: comparison is always false due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wtype-limits"

/*
C:\Users\Ger\Downloads\linnstrument-firmware\ls_extstorage.ino: In function 'void copyCalibrationV1(CalibrationX (*)[27][4], CalibrationX (*)[27][4], CalibrationY (*)[9][8], CalibrationYV1 (*)[9][8])':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_extstorage.ino:1213:75: error: 'max' was not declared in this scope
       (*calColsTarget)[i][j].minY = min(max((*calColsSource)[i][j].minY, 0), 4095);
                                                                           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_extstorage.ino:1213:82: error: 'min' was not declared in this scope
       (*calColsTarget)[i][j].minY = min(max((*calColsSource)[i][j].minY, 0), 4095);
                                                                                  ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_extstorage.ino: In function 'void copyCalibrationV2(CalibrationX (*)[27][4], CalibrationX (*)[27][4], CalibrationY (*)[9][8], CalibrationY (*)[9][8])':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_extstorage.ino:1230:75: error: 'max' was not declared in this scope
       (*calColsTarget)[i][j].minY = min(max((*calColsSource)[i][j].minY, 0), 4095);
                                                                           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_extstorage.ino:1230:82: error: 'min' was not declared in this scope
       (*calColsTarget)[i][j].minY = min(max((*calColsSource)[i][j].minY, 0), 4095);
                                                                                  ^
*/

#if 0

template <typename T>
static inline T min(T a, T b) {
	if (a <= b) {
		return a;
	} 
	return b;
}

template <typename T>
static inline T max(T a, T b) {
	if (a >= b) {
		return a;
	} 
	return b;
}

#else
	
#ifndef min
#define min(a, b)    ((a) <= (b) ? (a) : (b))
#endif
	
#ifndef max
#define max(a, b)    ((a) >= (b) ? (a) : (b))
#endif

#endif

// Return the number of elements in the given array
#define countof(a)          ( sizeof(a) / sizeof((a)[0]) )

#if 0
#define LS_PACKED           __attribute__ ((packed))
#else
#define LS_PACKED           
#endif

#endif // LS_COMPILER_TWEAKS_H
