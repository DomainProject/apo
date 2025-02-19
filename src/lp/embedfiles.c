#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *open_or_exit(const char *fname, const char *mode)
{
	FILE *f = fopen(fname, mode);
	if(f == NULL) {
		perror(fname);
		exit(EXIT_FAILURE);
	}
	return f;
}

void sanitize_symbol(const char *fname, char *symbol, size_t size)
{
	size_t i;
	/* Convert every character from fname to symbol, replacing dots with underscores */
	for(i = 0; i < size - 1 && fname[i] != '\0'; i++)
	{
		if(fname[i] == '.')
			symbol[i] = '_';
		else
			symbol[i] = fname[i];
	}
	symbol[i] = '\0';
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		fprintf(stderr, "USAGE: %s file1 [file2 ...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Open the output file "resources.c". All resource arrays and
	corresponding length symbols will be appended here. */
	FILE *out = open_or_exit("resources.c", "w");
	fprintf(out, "#include <stdlib.h>\n\n");

	/* Iterate through each resource file provided on the command line. */
	for(int argi = 1; argi < argc; argi++) {
		char symbol[256];
		/* Generate a valid C identifier by replacing dots with underscores.
		   For example, "ddm_v6.asp" becomes "ddm_v6_asp". */
		sanitize_symbol(argv[argi], symbol, sizeof(symbol));

		FILE *in = open_or_exit(argv[argi], "rb");
		fprintf(out, "const char %s[] = {\n\t", symbol);

		unsigned char buf[256];
		size_t nread;
		int byte_count = 0;
		/* Read the file in chunks and output each byte as a hexadecimal literal.
		   We limit the number of bytes per line to enhance readability. */
		while((nread = fread(buf, 1, sizeof(buf), in)) > 0) {
			for(size_t i = 0; i < nread; i++) {
				fprintf(out, "0x%02x,", buf[i]);
				if(++byte_count == 20) {
					fprintf(out, "\n\t");
					byte_count = 0;
				}
			}
		}
		if(byte_count > 0)
			fprintf(out, "\n");
		fprintf(out, "};\n");
		fprintf(out, "const size_t %s_len = sizeof(%s);\n\n", symbol, symbol);
		fclose(in);
	}

	fclose(out);
	return EXIT_SUCCESS;
}
