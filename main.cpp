/*
    Project:    Super Smash Bros File Editor
    By:         Nicholas Zonak
    Date:       2/22/2014
*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct fileHeader {
    unsigned int fileSize; //Size of the file
    unsigned int dataSize; //Size of the data
    unsigned int offsetCount;
    unsigned int FtDataCount;
    unsigned int secondSectionCount;
    unsigned int version;
    unsigned int unknown1;
    unsigned int unknown2;
} FileHeader;

typedef struct smashHeader {
    unsigned int attributeStart; //Offset in file for attributes
    unsigned int attributeEnd;
    int unknown1;
    int unknown2;
	int unknown3;
    unsigned int subactionStart; //offset for subactions
    unsigned int subactionEnd;
} SmashHeader;

typedef struct attribute {
	std::string name;
	float value;
} Attribute;

typedef struct attributes {
	Attribute *attributes;
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
std::string attrGetName(int cur);

int main(int argc, char* argv[]) {
    FILE *smashFile;
    FileHeader *fileHeader;
    SmashHeader *smashHead;
	Attributes *attr;
	std::string temp = (argc >= 2) ? argv[1] : "PlCa.dat";
	printf("File: %s\n", temp.c_str());
	printf("-------------------------------------------------------------------------------\n");
    smashFile = fopen(temp.c_str(), "rb+"); //Falcon is our testing dummy
    fileHeader = getFileHead(smashFile); //Get header data
    smashHead = getHeader(smashFile, fileHeader);
	attr = getAttributes(smashFile, smashHead);
    delete smashHead;
	delete fileHeader;
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

    printf("File Header\n_____________\nFilesize: %i\nDataSize: %i \noffsetCount: %i\nFtDataCount: %i\nsecondSectionCount: %i\nversion: %i\n", head->fileSize, head->dataSize, head->offsetCount, head->FtDataCount, head->secondSectionCount, head->version);
	printf("-------------------------------------------------------------------------------\n");
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
    printf("Ft Header\n___________________\n");
	printf("Attribute start (in Hex): 0x%08x \n", fileHead->attributeStart);
	printf("Subaction start (in Hex): 0x%08x \n", fileHead->subactionStart);
	printf("Subaction end   (in Hex): 0x%08x \n", fileHead->subactionEnd);
    return fileHead;
}

Attributes *getAttributes(FILE *file, SmashHeader *smash) {
	int offset = 0x20 + smash->attributeStart;
	Attributes *attr = new Attributes;
	int numOfElements = (smash->attributeEnd - smash->attributeStart) / 4;
	attr->attributes = new Attribute[numOfElements];
	float buff;
	int cur = 0;
	/*float falcSpeed = float_swap(109.0f);
	fseek(file, offset + 0x88, SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);
	falcSpeed = float_swap(1.08f);
	fseek(file,offset + 0x8C, SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);
	fseek(file, offset + (4 * 1), SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);
	fseek(file, offset + (4 * 2), SEEK_SET);
	fwrite(&falcSpeed, sizeof(float), 1, file);*/
	fseek(file, offset, SEEK_SET);
	while(cur < numOfElements) {
		fread(&buff, sizeof(float), 1, file);
		attr->attributes[cur].name = attrGetName(cur);
		attr->attributes[cur].value = float_swap(buff);
		cur++;
	}
	
	printf("-------------------------------------------------------------------------------\n");
	for (int i=0; i < numOfElements; i++) {
		printf("%s: %04f \n", attr->attributes[i].name.c_str(), attr->attributes[i].value);
	}
	printf("\n");
	
	return attr;
}

std::string attrGetName(int cur) {
	switch(cur * 4) {
		case 0x00:
			return "Walk Initial Velocity";
		case 0x04:
			return "Walk Acceleration?";
		case 0x08:
			return "Walk Max Velocity";
		case 0x0C:
			return "Slow Walk Max?";
		case 0x10:
			return "Mid Walk Point?";
		case 0x14:
			return "Fast Walk Min?";
		case 0x18:
			return "Friction / Stop Deccel";
		case 0x1C:
			return "Dash Initial Velocity";
		case 0x20:
			return "Dash & Run Acceleration A";
		case 0x24:
			return "Dash & Run Acceleration B";
		case 0x28:
			return "Dash & Run Terminal Velocity";
		case 0x2C:
			return "Run Animation Scaling";
		case 0x30:
			return "Run Acceleration?";
		case 0x38:
			return "Jump Startup Lag (Frames)";
		case 0x3C:
			return "Dash & Run Acceleration B";
		case 0x40:
			return "Jump V Initial Velocity";
		case 0x44:
			return "Ground to Air Jump Momentum Multiplier";
		case 0x48:
			return "Jump H Maximum Velocity";
		case 0x4C:
			return "Shorthop V initial Velocity";
		case 0x50:
			return "Air Jump Multiplier";
		case 0x54:
			return "Same as 0x04C ?";
		case 0x58:
			return "Number of Jumps";
		case 0x5C:
			return "Gravity";
		case 0x60:
			return "Terminal Velocity";
		case 0x64:
			return "Air Mobility A";
		case 0x68:
			return "Air Mobility B";
		case 0x6C:
			return "Max Aerial H Velocity";
		case 0x70:
			return "Air Friction";
		case 0x74:
			return "Fast Fall Terminal Velocity";
		case 0x78:
			return "0024?";
		case 0x7C:
			return "Jab 2 Window?";
		case 0x80:
			return "Jab 3 Window?";
		case 0x84:
			return "Frames to Change Direction on Standing Turn";
		case 0x88:
			return "Weight";
		case 0x8C:
			return "Model Scaling";
		case 0x90:
			return "Shield Size";
		case 0x94:
			return "Shield Break Initial Velocity";
		case 0x98:
			return "Rapid Jab Window";
		case 0xA8:
			return "Ledgejump Horizontal Velocity";
		case 0xAC:
			return "Ledgejump Vertical Velocity";
		case 0xB0:
			return "Item Throw Velocity (Alec's Favorite)";
		case 0xE0:
			return "Kirby Neutral+B Start Damage";
		case 0xE4:
			return "Normal Landing Lag";
		case 0xE8:
			return "N-Air Landing Lag";
		case 0xEC:
			return "F-Air Landing Lag";
		case 0xF0:
			return "U-Air Landing Lag";
		case 0xF8:
			return "D-Air Landing Lag";
		case 0xFC:
			return "Victory Screen Window Model Scaling";
		case 0x100:
			return "0x160?";
		case 0x104:
			return "Walljump H Velocity";
		case 0x108:
			return "Walljump V Velocity";
		case 0x14C:
			return "Bubble Ration?";
		case 0x160:
			return "Ice Traction?";
		case 0x16C:
			return "Camera Zoom Target Bone?";
		case 0x174:
			return "0x2D8?";
		default:
			return "Unknown";
	}
}