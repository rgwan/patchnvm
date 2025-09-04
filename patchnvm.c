#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int validate_chksum(FILE * fp)
{
	fseek(fp, 0, SEEK_SET);
	uint16_t value;
	uint16_t chksum = 0;
	int i;
	for (i = 0; i < 0x40; i++) {
		fread(&value, 2, 1, fp);
		chksum += value;
	}
	printf("validate chksum = %04x, %04x\n", value, chksum);
	if (chksum == 0xbaba)
		return 0;
	return 1;
}

void generate_chksum(FILE * fp)
{
	fseek(fp, 0, SEEK_SET);
	uint16_t value;
	uint16_t chksum = 0;
	int i;
	for (i = 0; i < 0x3F; i++) {
		fread(&value, 2, 1, fp);
		chksum += value;
	}
	chksum = 0xbaba - chksum;
	printf("chksum = %04x\n", chksum);
	fwrite(&chksum, 2, 1, fp);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr,
			"Usage\n"
			"\t %s: <nvm file> [address] [value to write]\n"
			"\t %s: <nvm file> [mac-address]\n", argv[0], argv[0]);
		return -1;
	}
	FILE *fp = fopen(argv[1], "rb+");
	int ret;
	if (!fp) {
		fprintf(stderr, "Cannot open file %s!\n", argv[1]);
		return -1;
	}
	ret = validate_chksum(fp);
	if (ret)
		fprintf(stderr, "Validate chksum failed\n");

	int chgaddr = -1;
	uint16_t chgvalue;
	if (argc == 4) {
		chgaddr = strtoul(argv[2], NULL, 16);
		chgvalue = strtoul(argv[3], NULL, 16);
		printf("Needs to change %02x to %04x\n", chgaddr, chgvalue);
		fseek(fp, 2 * chgaddr, SEEK_SET);
		fwrite(&chgvalue, 2, 1, fp);
	}
	int chgmac = 0;
	uint8_t mac_buf[6];
	if (argc == 3) {
		int j;
		char octet_buf[3];
		memset(octet_buf, 0x00, sizeof(octet_buf));
		if (strlen(argv[2]) != 12) {
			fprintf(stderr, "MAC format is not correct!\n");
			return -1;
		}
		for (j = 0; j < 6; j++) {
			memcpy(octet_buf, &argv[2][2 * j], 2);
			mac_buf[j] = strtoul(octet_buf, NULL, 16);
		}
		printf("New MAC address will be: ");
		for (j = 0; j < 6; j++)
			printf("%02x%s", mac_buf[j], j < 5 ? ":" : "");
		puts("");
		fseek(fp, 0, SEEK_SET);
		fwrite(mac_buf, 1, 6, fp);
		chgmac = 1;
	}

	int i;
	uint16_t value;
	fseek(fp, 0, SEEK_SET);
	for (i = 0; i < 0x40; i++) {
		if (i % 16 == 0) {
			printf("\n%02x: ", i);
		}
		fread(&value, 2, 1, fp);
		printf(" %04X", value);
		if (i == chgaddr || (chgmac && i < 3))
			printf("*");
	}
	puts("");
	if (chgaddr != -1 || chgmac) {
		generate_chksum(fp);

		ret = validate_chksum(fp);
		if (ret)
			fprintf(stderr, "Validate chksum failed\n");
	}
	fclose(fp);
}
