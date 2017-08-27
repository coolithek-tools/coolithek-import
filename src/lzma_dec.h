#ifndef __LZMA_DEC_H__
#define __LZMA_DEC_H__

#include <stdbool.h>
#include <lzma.h>
#include <string>

using namespace std;

class CLZMAdec
{
	private:
		bool init_decoder(lzma_stream *strm);
		bool decompress(lzma_stream *strm, const char *inname, FILE *infile, FILE *outfile);

	public:
		CLZMAdec() {};
//		~CLZMAdec();
		int decodeXZ(string inFile, string outFile);

};

#endif // __LZMA_DEC_H__
