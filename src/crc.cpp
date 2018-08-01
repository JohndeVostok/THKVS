#include "crc.h"

unsigned getCRC(unsigned char* buf, int nLength) {
	if (nLength < 1) {
		return 0xffffffff;
	}
	unsigned crc = 0;
	for (int i = 0; i < nLength; i++) {
		crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);  
	}
	crc = crc ^ 0xffffffff;
	return crc;  
}

unsigned getCRC(string &str) {
	return getCRC((unsigned char *)str.c_str(), str.size());
}

