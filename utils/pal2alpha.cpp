#include <iostream>

#include <stdlib.h>

#include <FreeImage.h>

int main(int argc, char* argv[])
{
	if(argc<3)
	{
		std::cout << "Usage: pal2alpha [transparency index] <input file(paletted bmp)> <output png 32 bit file>" << std::endl;
		return 1;
	}

	int transparency_index;
	const char* input_file;
	const char* output_file;

/*	for(int i=0; i<argc; i++)
		std::cout << "arg[" << i << "]=" << argv[i] << std::endl;
*/
	switch(argc)
	{
		case 3:
			transparency_index = -1;
			input_file = argv[1];
			output_file = argv[2];
			break;
		case 4:
			transparency_index = atoi(argv[1]);
			input_file = argv[2];
			output_file = argv[3];
			break;
	}

	std::cout << "transparency = " << transparency_index << std::endl
		<< "input = " << input_file << std::endl
		<< "output = " << output_file << std::endl;

	FIBITMAP *dib = FreeImage_Load(FIF_BMP, input_file, BMP_DEFAULT);
	if (dib) 
	{
		if(FreeImage_GetBPP(dib)==8)
		{
			if(transparency_index==-1)
			{
				BYTE real_index;
				FreeImage_GetPixelIndex(dib, 0, FreeImage_GetHeight(dib)-1, &real_index);
				transparency_index = real_index;
			}
			FreeImage_SetTransparentIndex(dib, transparency_index);
			// copy into a 32 bit picture
			FIBITMAP* newdib = FreeImage_ConvertTo32Bits(dib);
			std::cout << "Image type is transparent? " << FreeImage_IsTransparent(dib) << " bpp=" << FreeImage_GetBPP(newdib) << "idx=" << transparency_index << std::endl;
			if(newdib)
			{
				// process alpha channel:
				FreeImage_SetTransparent(newdib, true);
				std::cout << "Image type is transparent? " << FreeImage_IsTransparent(newdib) << std::endl;
				/*unsigned int color;
				if(transparency_index!=-1)
					color = reinterpret_cast<unsigned int*>(FreeImage_GetPalette(dib))[transparency_index];
				else
					color = */
				FreeImage_Save(FIF_PNG, newdib, output_file, 0);
				FreeImage_Unload(newdib);
			}
			else
				std::cout << "Can't convert into 32bit" << std::endl;
		}
		else
			std::cout << "Image is not a paletted one. Nothing to do" << std::endl;
        FreeImage_Unload(dib);
	}
	else
	{
		std::cout << "Couldn't open input file " << input_file << std::endl;
		return 1;
	}
	return 0;
}