/*
    Project:    Super Smash Bros File Editor
    By:         Nicholas Zonak
    Date:       2/22/2014
*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

typedef struct fileHeader {
    int fileSize; //Size of the file
    int dataSize; //Size of the data
    int offsetCount;
    int FtDataCount;
    int secondSectionCount;
    int version;
    int unknown1;
    int unknown2;
} FileHeader;

typedef struct smashHeader {
    int attributeStart; //Offset in file for attributes
    int attributeEnd;
    int unknown1;
    int unknown2;
	int unknown3;
    unsigned int subactionStart; //offset for subactions
    int subactionEnd;
} SmashHeader;

typedef struct attributes {
	float *attributes;
} Attributes;

int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

float float_swap(float value) {
    int temp =  swap_int32(*(int*)&value);
    return *(float*)&temp;
}

FileHeader *getFileHead(FILE *file);
SmashHeader *getHeader(FILE *file, FileHeader *header);
Attributes *getAttributes(FILE *file, SmashHeader *smash);

int main(int argc, char** argv) {
    FILE *smashFile;
    FileHeader *fileHeader;
    SmashHeader *smashHead;
	Attributes *attr;

    smashFile = fopen("PlGn.dat", "rb+"); //Falcon is our testing dummy
    fileHeader = getFileHead(smashFile); //Get header data
    smashHead = getHeader(smashFile, fileHeader);
	attr = getAttributes(smashFile, smashHead);
    delete smashHead;
	delete fileHeader;
	delete attr->attributes;
	delete attr;
    return 0;
}

FileHeader *getFileHead(FILE *file) {
    rewind(file); //Make sure it's at the start of the file
    FileHeader *head = new FileHeader;
    //Read the values from the file
    fread(&head->fileSize, sizeof(int), 1, file);
    fread(&head->dataSize, sizeof(int), 1, file);
    fread(&head->offsetCount, sizeof(int), 1, file);
    fread(&head->FtDataCount, sizeof(int), 1, file);
    fread(&head->secondSectionCount, sizeof(int), 1, file);
    fread(&head->version, sizeof(int), 1, file);
    fread(&head->unknown1, sizeof(int), 1, file);
    fread(&head->unknown2, sizeof(int), 1, file);

    //Convert the values to Little Endian
    head->fileSize = swap_int32(head->fileSize);
    head->dataSize = swap_int32(head->dataSize);
    head->offsetCount = swap_int32(head->offsetCount);
    head->FtDataCount = swap_int32(head->FtDataCount);
    head->secondSectionCount = swap_int32(head->FtDataCount);
    head->version = swap_int32(head->version);

    printf("Filesize: %i\nDataSize: %i \noffsetCount: %i\nFtDataCount: %i\nsecondSectionCount: %i\nversion: %i\n", head->fileSize, head->dataSize, head->offsetCount, head->FtDataCount, head->secondSectionCount, head->version);
    //Nick Note: I don't give a shit abbout the unkown values

    return head;
}

SmashHeader *getHeader(FILE *file, FileHeader *header) {
    smashHeader *fileHead = new SmashHeader;
    int offset; //the calculated offset to the information containing Attribute and subaction data
    offset = 0x20 + header->dataSize + header->offsetCount * 4; 
    fseek(file, offset, SEEK_SET);
    int a;
    //This reads in the Melee header. Contains information for attributes and subactions
    fread(&a, sizeof(int), 1, file);
    fseek(file, swap_int32(a) + 0x20, SEEK_SET);
    fread(&a, sizeof(int), 1, file);
    fileHead->attributeStart = swap_int32(a);
	fread(&a, sizeof(int), 1, file);
    fileHead->attributeEnd = swap_int32(a);
	fread(&a, sizeof(int), 1, file);
    fileHead->unknown1 = swap_int32(a);
	fread(&a, sizeof(int), 1, file);
    fileHead->subactionStart = swap_int32(a);
	fread(&a, sizeof(int), 1, file);
    fileHead->unknown2 = swap_int32(a);
	fread(&a, sizeof(int), 1, file);
    fileHead->subactionEnd = swap_int32(a);
	fread(&a, sizeof(int), 1, file);
    fileHead->unknown3 = swap_int32(a);
    printf("Attribute start (For Falcon, should be 3754): 0x%08x \n", fileHead->attributeStart);
    return fileHead;
}

Attributes *getAttributes(FILE *file, SmashHeader *smash) {
	int offset = 0x20 + smash->attributeStart;
	Attributes *attr = new Attributes;
	int numOfElements = (smash->attributeEnd - smash->attributeStart) / 4;
	attr->attributes = new float[numOfElements];
	float buff;
	int cur = 0;
	float falcSpeed = float_swap(109.0f);
	fseek(file, offset + 0x88, SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);
	falcSpeed = float_swap(1.08f);
	fseek(file,offset + 0x8C, SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);
	/*fseek(file, offset + (4 * 1), SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);
	fseek(file, offset + (4 * 2), SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);*/
	fseek(file, offset, SEEK_SET);
	while(cur < numOfElements) {
		fread(&buff, sizeof(float), 1, file);
		attr->attributes[cur] = float_swap(buff);
		cur++;
	}
	
	for (int i=0; i < numOfElements; i++) {
		printf("%f \n", attr->attributes[i]);
	}
	printf("\n");
	
	return attr;
}