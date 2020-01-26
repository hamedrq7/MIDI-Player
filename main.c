#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "beep.h"

//cd Desktop/phase3/TestSpace
//compile with : gcc main.c beep.o sintable.o -lasound

unsigned long long int Tempo = 55555;

//Prototypes : 
void ReadingHeader(FILE *infile);
int ReadMIDI(int type, int byte_length, FILE *infile, int index_track);
int changeEndian(int a);
int ReadDeltaTime(FILE *infile, unsigned long long int *a);
int ReadEvents(FILE *infile, unsigned long long int buff);

double getFreq(int kkkkkkk){

    float freq[] = {16.35,17.32,18.35,19.45,20.60,21.83,23.12,24.50,16.35,27.50,29.14,30.87,32.70,34.65,36.71,38.89,41.20,43.65,46.25,49.00,51.91,55.00,58.27,61.74,65.41,69.30,73.42,77.78,82.41,87.31,92.50,98.00,103.83,110.00,116.54,123.47,130.81,138.59,146.83,155.56,164.81,174.61,185.00,196.00,207.65,220.00,233.08,246.94,261.63,277.18,293.66,311.13,329.63,349.23,369.99,392.00,415.30,440.00,466.16,493.88,523.25,554.37,587.33,622.25,659.25,698.46,739.99,783.99,830.61,880.00,932.33,987.77,1046.50,1108.7,1174.6,1244.5,1318.51,1396.91,1479.98,1567.98,1661.22,1760.00,1864.66,1975.53,2093.00,2217.46,2349.32,2489.02,2637.02,2793.83,2959.96,3135.96,3322.44,3520.00,3729.31,3951.07,4186.01,4434.92,4698.63,4978.03,5274.04,5587.65,5919.91,6271.93,6644.88,7040.00,7458.62,7902.13};
    return freq[kkkkkkk - 1];
}

struct Track {
	char type[11];
	int length;
} track[200];


struct note
{
	//Delay  ? ? ? ? delta time 9n : 
/**/	unsigned long long int delay_prep;

												//delta time 9n ro k dorost kard bad , be onvane delay tooye in beriz : 
	int time;

/**/	double delay;

/**/	int channel; // : 8nnn , 9nnn

/**/	int key; // : 0kkk kkkk

/**/	int velocity; // : 0vvv vvvvv 

/**/	unsigned long long int tempo;

/**/	unsigned long long int dur_prep; //delta time of 8n

	//ke az delta time 8n be dast miad : 
/**/	double dur;

/**/	double frequency;

} note[10000];

int note_index = 0;
//notestatus = 0 (off)
//notestatus = 1 (On)
int notestatus = 0;
struct Header {
	char type[11];
	int length;
	int format;
	int ntracks;
	int division;
} Hchunk;



int main ()
{
	FILE *infile;
	infile = fopen("forelise.mid", "rb");
	if(infile == NULL) {
		printf("MIDI File Not Found\n");
		return 0;
	}
	ReadingHeader(infile);
	for(int ii = 0; ii < Hchunk.ntracks; ii++) 
	{

		//Reading TYPE : 
		ReadMIDI(5, 4, infile, ii);
		ReadMIDI(6, 4, infile, ii);

		note_index = 0;
		notestatus = 0;
		int flag;
		unsigned long long int buff;
		do {
			if(!ReadDeltaTime(infile, &buff)) break;

			flag = ReadEvents(infile, buff);

			//For handling invalid events : 

			/*
			flag = ReadEvents(infile, buff);

			//if flag == 85 then its invalid event , so we keep reading byte by byte until we get to a valid event : 
			if(flag == 85) {
				do {

					flag = ReadEvents(infile, buff);

				} while (flag == 85);
			}
			*/
		} while(flag != -1);


		printf("note_index : %d\n", note_index);

		for(int i = 0; i < note_index; i++)
		{
	        printf("found %f %llu %llu\n" ,  note[i].frequency , note[i].tempo , note[i].dur_prep);
	        /*
	        //Calculating duration ?: : : : 
	        note[i].dur = (60*1000000) / (double)note[i].tempo;
	        note[i].dur /= (double)100;
	        note[i].dur /= (double)50;
	        if(note[i].dur_prep < 10000) {
	            note[i].dur *= note[i].dur_prep*175;
	        } else{
	            note[i].dur *= note[i].dur_prep;
	        }
	        */

	        //printf("asdadsa%d\n", Hchunk.division);
	        note[i].dur = (60*1000000) / (double)note[i].tempo;

       		note[i].dur *= Hchunk.division;

      		double MSPT = 60000 / (double)note[i].dur;

     		MSPT *= note[i].dur_prep;

     		note[i].dur = MSPT;

	        printf("DURATION IS %lf\n" , note[i].dur);

	        //calculating delay :  :: : 
	        note[i].delay = (60*1000000) / (double)note[i].tempo;
	        note[i].delay /= (double)100;
	        note[i].delay /= (double)50;

	        if(note[i].delay_prep < 10000) {
	            note[i].delay *= note[i].delay_prep*175;
	        } else{
	            note[i].delay *= note[i].delay_prep;
	        }

	        double delay = (double) note[i].delay / 1000;
	        sleep((double)delay / 2);
	        printf("DELAY IS %lf(  %llu  )\n" , delay, note[i].delay_prep);
	        printf("Playing ...\n");
	        beep(note[i].frequency, note[i].dur+200);
	        printf("\n");
		}
	}
}

int ReadEvents(FILE *infile, unsigned long long int buff)
{
	unsigned char first_byte;
	fread(&first_byte, 1, 1, infile);
	unsigned char sign = first_byte >> 4;

	//New Line : : : : : :: : : : :: : 
	//note[note_index].dur_prep += buff;

	//Setting note off : 
	if(sign == 8) // 8
	{

		//initailzing channel : 
		int channel_temp;
		channel_temp = first_byte - 0x80;

		if(notestatus == 1) //if note is on 
		{
			char kkkkkkk;
			fread(&kkkkkkk, 1, 1, infile);

			char vvvvvvv;
			fread(&vvvvvvv, 1, 1, infile);
			
			if(kkkkkkk == note[note_index].key && channel_temp == note[note_index].channel) //if this note off event was for the current note on event that is currently playing : 
			{

				//KInd of did it in New Line(155)
				note[note_index].dur_prep = buff; 

				printf("%x %x %x ", first_byte, kkkkkkk, vvvvvvv);
				note[note_index].tempo = Tempo;
				printf("Note Off\n");
				//Set off : 
				notestatus = 0;
				note_index++;

				//Setting next note index to zero : 
				note[note_index].dur_prep = 0;
			}
			else 
			{
				//ignoring beacause : ba inke alan note On hastesh , va ma note off darim , ama in note off marboot be oon evente note on ee ke dar hale pakhshe nist
				printf("Ignoring : %x %x %x (notestatus doesnt change)\n", first_byte, kkkkkkk, vvvvvvv);
			}
		}
		else //never happens any way 
		{
			char kkkkkkk;
			fread(&kkkkkkk, 1, 1, infile);
			
			char vvvvvvv;
			fread(&vvvvvvv, 1, 1, infile);

			printf("Ignoring : %x %x %x (notestatus doesnt change)\n", first_byte, kkkkkkk, vvvvvvv);
		}
		
	}
	//Setting note on : 
	else if(sign == 9) // 9
	{
		//initailzing channel : 
		int channel_temp;
		channel_temp = first_byte - 0x90;

		if(notestatus == 0) //if note is Off 
		{
			//initailzing channel to struct note:
			note[note_index].channel = channel_temp;

			char kkkkkkk;
			fread(&kkkkkkk, 1, 1, infile);
			note[note_index].key = kkkkkkk;

			note[note_index].delay_prep = buff; 

			float frequency = getFreq(kkkkkkk);
			note[note_index].frequency = frequency;
			//Set on : 
			notestatus = 1;

			char vvvvvvv;
			fread(&vvvvvvv, 1, 1, infile);
			note[note_index].velocity = vvvvvvv;
			printf("%x %x %x ", first_byte, kkkkkkk, vvvvvvv);
			printf("Note On\n");
		} 
		else // note is On (this means there is a note playing) and another note is about to play on top of that but we cant :(
		{
			//Just read 
			char kkkkkkk;
			fread(&kkkkkkk, 1, 1, infile);

			char vvvvvvv;
			fread(&vvvvvvv, 1, 1, infile);
			note[note_index].velocity = vvvvvvv;
			printf("FORMAT 1 :%x %x %x (notestatus doesnt change)\n", first_byte, kkkkkkk, vvvvvvv);

		}
	}

	//Meta Event or sysex events :
	else if(sign == 15) // F
	{
		

		////Meta events : 
		if(first_byte == 0xFF) {

			unsigned char type;
			fread(&type, 1, 1, infile);

			//Set Tempo :
	        if(type == 81) //FF 51 :
	        {
	            unsigned char buff;
	            fread(&buff, 1, 1, infile);
	            int length2 = buff;

	            unsigned char buff2[4];
	            fread(buff2, 3, 1, infile);

	            int tempo_buff = buff2[0] << 16 | buff2[1] << 8 | buff2[2];
	            printf("(%x %x)Set Tempo to %llu microseconds per quarter note\n", first_byte, type, Tempo);
	            Tempo = tempo_buff;
	            printf("(%x %x)Set Tempo to %llu microseconds per quarter note\n", first_byte, type, Tempo);
	        }		

	        //End of track : 
	        else if(type == 47) //FF 2F
	        {
	        	char bf;
	        	fread(&bf, 1 , 1, infile);
	        	printf("\n\n(%x %x %x)End of track\n\n\n", first_byte, type, bf);
	        	return -1;
	        }

	        ///Bikhoodia : 
	        else 
	        {
	        	if(type == 0) { //FF 00
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Sequence Number Event\n", first_byte, type);
	        	}
	        	else if(type == 1) { //FF 01
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Text Event : %s\n", first_byte, type, buff2);
	        	}
				else if(type == 2) { //FF 02
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Copyright Notice Event : %s\n", first_byte, type, buff2);
	        	}
	        	else if(type == 3) { //FF 03
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Sequence/Track Name Event : %s\n", first_byte, type, buff2);
	        	}
	        	else if(type == 4) { //FF 04
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2+3];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Instrument Name Event : %s\n", first_byte, type, buff2);
	        	}
	        	else if(type == 5) { //FF 05
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Lyric Event : %s\n", first_byte, type, buff2);
	        	}
	        	else if(type == 6) { //FF 06
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Marker Event : %s\n", first_byte, type, buff2);
	        	}
	        	else if(type == 7) { //FF 07
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Cue Point Event : %s\n", first_byte, type, buff2);
	        	}
	        	else if(type == 32) { //FF 20
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) MIDI Channel Prefix Event\n", first_byte, type);
	        	}
	        	else if(type == 84) { //FF 54
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) SMPTE Offset Event\n", first_byte, type);
	        	}
	        	////////////////////////////////////////////////////////////  Usefull Perhaps : 
	        	else if(type == 88) { //FF 58
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Time Signature Event\n", first_byte, type);
	        	}
	        	else if(type == 89) { //FF 59
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Key Signature Event\n", first_byte, type);
	        	}
	        	else if(type == 127) { //FF 7F
	        		unsigned char buff;
		        	fread(&buff, 1, 1, infile);
		        	int length2 = buff;
		            unsigned char buff2[length2];
		            fread(buff2, length2, 1, infile);
		            buff2[length2] = '\0';
		            printf("(%x %x) Sequencer Specific Meta Event\n", first_byte, type);
	        	}
       		}
        }
        //Sysex events : 
        else if(first_byte == 0xF0 || first_byte == 0xF7)
        {
        	unsigned char buff;
		    fread(&buff, 1, 1, infile);
		    int length2 = buff;
	        unsigned char buff2[length2];
		    fread(buff2, length2, 1, infile);
		    buff2[length2] = '\0';
		    printf("(%x) System Exclusive Event(sysex event)\n", first_byte);
        }
	}


	/////Bikhodi : 

	//MIDI Control Events or MIDI Channel Events or <MIDI event> : 
	else if(sign == 10) { // An 
		unsigned char channel_an;
		channel_an = first_byte - 0xA0;
		unsigned char kkkkkkk;
		fread(&kkkkkkk, 1, 1, infile);
		unsigned char ppppppp;
		fread(&ppppppp, 1, 1, infile);
		printf("Key Pressure Event :\tChannel = %x\tKey/Note Number = %x\tKey = %x\n", channel_an, kkkkkkk, ppppppp);
	} 
	
	else if(sign == 11) // Bn
	{
		unsigned char channel_bn;
		channel_bn = first_byte - 0xB0;
		unsigned char Data_byte_1;
		fread(&Data_byte_1, 1, 1, infile);
		unsigned char ttttttt;
		fread(&ttttttt, 1, 1, infile);
		if(Data_byte_1 == 0x00) {
			printf("Bank Select Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x01) {
			printf("Modulation Wheel Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x02) {
			printf("Breath Controller Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x04) {
			printf("Foot Controller Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x05) {
			printf("Portamento Time Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x06) {
			printf("Data Entry Slider Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x07) {
			printf("Main Volume Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x08) {
			printf("Balance Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x0A) {
			printf("Pan Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x0B) {
			printf("Expression Controller Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x0C || Data_byte_1 == 0x0D) {
			printf("Effect Control Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 >= 0x10 && Data_byte_1 <= 0x13) {
			printf("General-Purpose Controllers Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		///Name of these events ? ? ? ? ? 
		else if(Data_byte_1 >= 0x20 && Data_byte_1 <= 0x3F) {
			printf("LSB for controllers Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x40) {
			printf("Damper Pedal (Sustain Pedal) Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x41) {
			printf("Portamento Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x42) {
			printf("Sostenato Pedal Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x43) {
			printf("Soft Pedal Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x44) {
			printf("Legato Footswitch Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x45) {
			printf("Hold Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x45) {
			printf("Hold Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 >= 0x46 && Data_byte_1 <= 0x4F) {
			printf("Sound Controller Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 >= 0x50 && Data_byte_1 <= 0x53) {
			printf("General-Purpose Controllers Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x54) {
			printf("Portamento Control Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 >= 0x5B && Data_byte_1 <= 0x5F) {
			printf("Effects Depth Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x60 || Data_byte_1 == 0x61) {
			printf("Data Increment Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x62 || Data_byte_1 == 0x63) {
			printf("Non-Registered Parameter Number Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 == 0x64 || Data_byte_1 == 0x65) {
			printf("Registered Parameter Number Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}
		else if(Data_byte_1 >= 0x79 && Data_byte_1 <= 0x7F) {
			printf("Mode Messages Event :\tChannel = %x\tController Type = %x\n", channel_bn, ttttttt);
		}


	}
	else if(sign == 12) { // Cn
		unsigned char channel_cn;
		channel_cn = first_byte - 0xC0;
		unsigned char mmmmmmm;
		fread(&mmmmmmm, 1, 1, infile);
		printf("Program Change (=select instrument) Event :\tChannel = %x\tProgram Number = %x\n", channel_cn, mmmmmmm);
	}
	else if(sign == 13) // Dn
	{
		unsigned char channel_dn;
		channel_dn = first_byte - 0xD0;
		unsigned char rrrrrrr;
		fread(&rrrrrrr, 1, 1, infile);
		printf("Channel Aftertouch or Channel Pressure Event :\tChannel = %x\tAmount = %x\n", channel_dn, rrrrrrr);
	}
	else if(sign == 14) // E
	{
		unsigned char channel_en;
		channel_en = first_byte - 0xE0;
		unsigned char fffffff;
		fread(&fffffff, 1, 1, infile);
		unsigned char ccccccc;
		fread(&ccccccc, 1, 1, infile);
		printf("Pitch Bend Event :\tChannel = %x\tvalue LSB = %x\tvalue MSB = %x\n", channel_en, fffffff, ccccccc);

	}	
	else {
		printf("Event Not Found ( %x is invalid event)\n", first_byte);
		//** event was not found :
		return 85;
	}

	///event read succesfully
	return 1;
}

int ReadDeltaTime(FILE *infile, unsigned long long int *a) {
	int END;
	unsigned long long int temp[3];
	unsigned char temp_buff;
	END = fread(&temp_buff, 1, 1, infile);
	if(END <= 0) return 0; 
	temp[0] = temp_buff;
	if(temp[0] > 127)
	{
		fread(&temp_buff, 1, 1, infile);
		temp[1] = temp_buff;
		if(!(temp[1] > 127)) 
		{
			temp[0] = temp[0] - 128;
			temp[0] = temp[0] << 7;
			*a = temp[1] | temp[0];
			//printf("(Dl.Time : %lli) ", track[*event_index].deltatime);
		}
		else 
		{
			fread(&temp_buff, 1, 1, infile);
			temp[2] = temp_buff;
			temp[1] = temp[1] - 128;
			temp[1] = temp[1] << 7;
			unsigned long long int StillNotDelta = temp[2] | temp[1];
			temp[0] = temp[0] - 128;
			temp[0] = temp[0] << 14;
			*a = StillNotDelta | temp[0];
		}
	}
	else 
	{ 
		*a = temp[0];
		//printf("(Dl.Time : %lli) ", track[*event_index].deltatime);
	}
	return 1;
}

void ReadingHeader (FILE *infile)
{
	ReadMIDI(0, 4, infile, 0); //Calling Function for reading HeaderType
	ReadMIDI(1, 4, infile, 0); //Calling Function for reading Length
	ReadMIDI(2, 2, infile, 0); //Calling Function for reading Format
	ReadMIDI(3, 2, infile, 0); //Calling Function for reading NumberOfTracks
	ReadMIDI(4, 2, infile, 0); //Calling Function for reading Division
}


int ReadMIDI(int type, int byte_length, FILE *infile, int index_track) {
	if(type == 0) { //Reading Header Type
		fread(Hchunk.type, byte_length, 1, infile);
		printf("HeaderType : %s\n", Hchunk.type);
	}
	else if(type == 1) { //Reading Length //length 4 byte hast , pas mitonim ba int daryaft konim va baad ba change endian dorost konim
		fread(&Hchunk.length, byte_length, 1, infile);
		//Ba Fraze Inke LittleEndian Bashe SYSTEM :
		Hchunk.length = changeEndian(Hchunk.length);
		printf("Length : %d\n", Hchunk.length);
	}
	else if(type == 2) { //Reading format
		unsigned char buffer[byte_length];
		fread(buffer, byte_length, 1, infile);
		//Ba Fraze Inke LittleEndian Bashe SYSTEM :
		Hchunk.format = buffer[1] | buffer[0] << 8;
		printf("Format : %d\n", Hchunk.format);
	}
	else if(type == 3) { //Reading tracks
		unsigned char buffer[byte_length];
		fread(buffer, byte_length, 1, infile);
		//Ba Fraze Inke LittleEndian Bashe SYSTEM :
		Hchunk.ntracks = buffer[1] | buffer[0] << 8;
		printf("NumberOfTracks : %d\n", Hchunk.ntracks);

	}
	else if(type == 4) { //Reading division
		unsigned char buffer[byte_length];
		fread(buffer, byte_length, 1, infile);
		//Ba Fraze Inke LittleEndian Bashe SYSTEM :
		Hchunk.division = buffer[1] | buffer[0] << 8;
		printf("Division : %d\n", Hchunk.division);
	}
	///////////////
	//Track Chunk : 
	else if(type == 5) { //Reading Track Chunk type
		fread(track[index_track].type, byte_length, 1, infile);
		printf("TrackType : %s\n", track[index_track].type);
	}
	else if(type == 6) { //Reading Length //length 4 byte hast , pas mitonim ba int daryaft konim va baad ba change endian dorost konim
		fread(&track[index_track].length, byte_length, 1, infile);
		//Ba Fraze Inke LittleEndian Bashe SYSTEM :
		track[index_track].length = changeEndian(track[index_track].length);
		printf("Length Of Track : %d\n", track[index_track].length);
	}
}


int changeEndian(int a) {
	return (((a>>24) & 0x000000ff | ((a>>8) & 0x0000ff00) | ((a<<8) & 0x00ff0000) | ((a<<24) & 0xff000000)));
}
