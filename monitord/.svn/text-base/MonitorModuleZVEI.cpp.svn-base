// MyMonModuleZVEI.cpp: Implementierung der Klasse MonitorModuleZVEI.
//
// Edited 02/2007 - demod rewritten, fully functional but to be tested "in the wild":
// Martin Diedrich (martin@mdiedrich.de)
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include "math.h"
#include "time.h"
#include "convert.h"

#include "MonitorModuleZVEI.h"
#include "MonitorLogging.h"

using namespace std;

#ifdef WIN32
	#include <vector>
	#include <algorithm>
	#include <functional>
#endif

#ifdef _DEBUG
#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
#endif

#ifdef WIN32
#ifndef M_PI
const float M_PI = 3.14159265358979 ;
#endif
#endif

#define COS(x) costabf[(((x) >> 6) & 0x3ffu)]
#define SIN(x) COS((x) + 0xc000)
#define BLOCKNUM 4

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

int MonitorModuleZVEI::PHINC(int x)
{
 return (x * 0x10000 / FREQ_SAMP) ;
}

/**
 * @brief constructor, initializing most of our little ZVEI universe ;)
 * @param sampleRate sample rate of data from soundcard to be analyzed by demod()
 */
MonitorModuleZVEI::MonitorModuleZVEI(int sampleRate,XMLNode* pConfig)
{
	unsigned int i ;
	
	debugmodus = getNodeInt(*pConfig,"debugmodus",0);
	int gm900 = getNodeInt(*pConfig,"gm900",0); // ONLY FOR GM900 AT FHD OSTERODE
	squelch = getNodeInt(*pConfig,"squelch",51);
	squelch = squelch/100;

	m_lpszName="ZVEI" ;

	FREQ_SAMP=sampleRate ;
	BLOCKLEN=sampleRate / 100 ;

	// Definition der zu betrachtenden Frequenzen
	zvei_freq[0]=PHINC(2400) ; // Ziffer 0
	if(gm900 == 1) {
		zvei_freq[0]=PHINC(2410) ; // Ziffer 0 for slightly too high tone by motorola gm900 in use at fhd osterode
	}
	zvei_freq[1]=PHINC(1060) ; // Ziffer 1
	zvei_freq[2]=PHINC(1160) ; // Ziffer 2
	zvei_freq[3]=PHINC(1270) ; // Ziffer 3
	zvei_freq[4]=PHINC(1400) ; // Ziffer 4
	zvei_freq[5]=PHINC(1530) ; // Ziffer 5
	zvei_freq[6]=PHINC(1670) ; // Ziffer 6
	zvei_freq[7]=PHINC(1830) ; // Ziffer 7
	zvei_freq[8]=PHINC(2000) ; // Ziffer 8
	zvei_freq[9]=PHINC(2200) ; // Ziffer 9
	zvei_freq[10]=PHINC(2800) ; // A = N / Spezialfall Gruppenruf
	zvei_freq[11]=PHINC(810) ; // C
	zvei_freq[12]=PHINC(675) ; // Sirenendoppelton I
	zvei_freq[13]=PHINC(1240) ; // Sirenendoppelton II
	zvei_freq[14]=PHINC(2600) ; // E = W (Wiederholungs- und Melderweckton)
	zvei_freq[15]=PHINC(0) ; // Stille/Rauschen/keine eindeutige Frequenz
	zvei_freq[16]=PHINC(1860) ; // Sirenendoppelton III
	zvei_freq[17]=PHINC(825) ; // Sirenendoppelton IV
	zvei_freq[18]=PHINC(2280) ; // Sirenendoppelton V
	zvei_freq[19]=PHINC(1010) ; // Sirenendoppelton VI

	// Definition der zu betrachtenden Frequenzen
	zvei_character[0]='0' ; // Ziffer 0
	zvei_character[1]='1' ; // Ziffer 1
	zvei_character[2]='2' ; // Ziffer 2
	zvei_character[3]='3' ; // Ziffer 3
	zvei_character[4]='4' ; // Ziffer 4
	zvei_character[5]='5' ; // Ziffer 5
	zvei_character[6]='6' ; // Ziffer 6
	zvei_character[7]='7' ; // Ziffer 7
	zvei_character[8]='8' ; // Ziffer 8
	zvei_character[9]='9' ; // Ziffer 9
	zvei_character[10]='A' ; // A = N / Spezialfall Gruppenruf
	zvei_character[11]='C' ; // C
	zvei_character[12]='S' ; // Sirenendoppelton I
	zvei_character[13]='S' ; // Sirenendoppelton II
	zvei_character[14]='W' ; // E = W (Wiederholungs- und Melderweckton)
	zvei_character[15]='-' ; // Stille/Rauschen/keine eindeutige Frequenz
	zvei_character[16]='S' ; // Sirenendoppelton III
	zvei_character[17]='S' ; // Sirenendoppelton IV
	zvei_character[18]='S' ; // Sirenendoppelton V
	zvei_character[19]='S' ; // Sirenendoppelton VI

	// initializing our matrixes -> zeroing
	for (i=0; i <= sizeof(zvei_freq)/sizeof(zvei_freq[0]); i++) {
		ph[i]=0;
	}
	for (i=0; i < 4  ;i++) {
		energy[i]=0;
	}
	for (i=0; i< 4; i++) {
		for (unsigned int j=0; j <= 2*sizeof(zvei_freq)/sizeof(zvei_freq[0]); j++) {
			tenergy[i][j]=0;
		}
	}

	blkcount=0 ;

	folge_position = 0; // gibt die aktuelle Position bei der Erkennung an, in gewisser Weise der "Zustandsautomat"
	timeout = 0; // Zu lange Pause nach Tonfolge -> triggert Ausgabe und Resets
	pause_length = 0; // Laenge der bisher gemessenen Pause, reset bei 100
	tone_count = 0; // wie oft wurde der ton erkannt?
	seven_count = 0;
	siren_count = 0;
	maxlength = sizeof(zvei_folge)/sizeof(zvei_folge[0]); // Laenge der n-Tonfolge. Normalerweise 5. Es gab einen Request fuer 7.

	/*
	* Format des Arrays fount_tones:
	* [0] - energiereichste Frequenz, [1] - zweitenergier. Freq., [2] - drittenergier. Freq., [3] - Energie I, [4] - Energie II - [5] - Energie III,
	* [6] - Totale Energie; damit stehen drei Toene und Energien zur Verfuegung fuer Auswertung und Filter
	*/
	found_tones = new int[7]; // Rueckgabe der process_block()-Methode
	for (i=0; i < 7; i++) {
		for(int j=0; j<7; j++) {
			detected_seven[i][j] = -1;
		}
	}

	for(i=0; i < maxlength; i++) {
		zvei_folge[i] = -2;
	}

	for (i = 0; i < COSTABSIZE; i++) {
		costabf[i] = (float) cos(M_PI*2.0*i/COSTABSIZE);
	}

}

MonitorModuleZVEI::~MonitorModuleZVEI()
{

}

/**
 * @brief demodulates raw sounddata from card to ZVEI-Tonfolgen
 * @param buffer pointer to sound buffer
 * @param length length of that buffer
 * @author Martin Diedrich (mdi)
 * @date 11/2007
 */
void MonitorModuleZVEI::demod(float *buffer, int length)
{
	unsigned int i ;
	float s_in=0;
	short int last_zvei_last_character = -2;

	// uebernommen aus dem vorherigen code
	for (; length > 0; length--, buffer++) {
		s_in = *buffer;
		energy[0] += fsqr(s_in);


/*	Berechnen und Aufaddieren der Spalte 0 der tenergy-Matrix
 *	mit 220 (BLOCKLEN) aufeinanderfolgenden Bufferwerten:
 *	Zeilen  0-17: Cos-Werte, Zeilen 18-35: Sin-Werte	*/
		for (i = 0; i < sizeof(zvei_freq)/sizeof(zvei_freq[0]); i++) {
			tenergy[0][i] += COS(ph[i]) * s_in;
			tenergy[0][i + sizeof(zvei_freq)/sizeof(zvei_freq[0])] += SIN(ph[i]) * s_in;
			ph[i] += zvei_freq[i];
		}

		if ((blkcount--) <= 0) {
			blkcount = BLOCKLEN;
			found_tones = process_block(found_tones);

			memcpy(detected_seven[seven_count], found_tones, sizeof(int)*7);

			int f = fuzzyseven();

			if( debugmodus > 5 && ((f >= 0 && f <= 11) || f == 14)) {
				cout << " -> " << f << " -> " << zvei_character[f] << endl;
			}

			// ZVEI digit or repeating tone found, not end of ZVEI quintett
			if(((f >= 0 && f <= 11) || f == 14) && folge_position != maxlength) {
				if(folge_position != 0) {
					if(f != zvei_folge[folge_position-1]) {
						zvei_folge[folge_position] = f;
						folge_position++;
					}
				} else {
						zvei_folge[folge_position] = f;
						folge_position++;
				}
				pause_length = 0;
			}

			// wakeup found shortly before maximum pause time
			if(f == 14 && folge_position == maxlength && pause_length > 52) {
				if(zvei_ok()) {
					if(debugmodus > 2) {
						cout << endl;
						for(i=0; i<5; i++) {
							cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
						}
						cout << " (Melderausloesung)" << endl << endl;
					}
					std::string numString="";
					for(unsigned int counter=0;counter<maxlength;counter++) {
						// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
						numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
					}
					DisplayResult(numString,1,"Melderausloesung");
				}
				for(i=0; i<maxlength; i++) {
					zvei_folge[i] = -2;
				}
				folge_position = 0;
				pause_length = 0;
				siren_count = 0;
			}

			// normal pause handling
			if ((f == 15 || f == -1) && folge_position != maxlength ) {
				pause_length++;
				if(pause_length == 6) {
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					folge_position = 0;
					pause_length = 0;
				}
				if(debugmodus > 5) {
					cout << " Pause: " << pause_length;
				}
			}

			// waiting for siren or wakeup - or reacting on timeout
			if ((f == 15 || f == -1) && folge_position == maxlength ) {
				pause_length++;
				if(debugmodus > 5) {
					cout << " Pause: " << pause_length;
				}
				if(pause_length == 64) { // timeout
					if(zvei_ok()) {
						if(debugmodus > 2) {
							cout << endl;
							for(i=0; i<maxlength; i++) {
								cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
							}
							cout << " (Timeout)" << endl << endl;
						}
						std::string numString="";
						for(unsigned int counter=0;counter<maxlength;counter++) {
							// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
							numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
						}
						DisplayResult(numString,0,"unklare Ausloesung");
					}
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					last_zvei_last_character = -2;
					folge_position = 0;
					pause_length = 0;
					siren_count = 0;
				}
			}

			// got siren tone 12/13 - fire
			if ( ((found_tones[0] == 12 && found_tones[1] == 13) || (found_tones[1] == 12 && found_tones[0] == 13)) && folge_position == maxlength) {
				siren_count++;
				pause_length = 0;
				if(siren_count > 20) {
					if(zvei_ok()) {
						if(debugmodus > 2) {
							cout << endl;
							for(i=0; i<maxlength; i++) {
								cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
							}
							cout << " (Siren2)" << endl << endl;
						}
						std::string numString="";
						for(unsigned int counter=0;counter<maxlength;counter++) {
							// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
							numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
						}
						DisplayResult(numString,2,"Sirenenausloesung: Feueralarm");
					}
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					folge_position = 0;
					siren_count = 0;
				}
			}

			// got siren tone 12/19 - ZVS-Entwarnung
			if ( ((found_tones[0] == 12 && found_tones[1] == 19) || (found_tones[1] == 12 && found_tones[0] == 19)) && folge_position == maxlength) {
				siren_count++;
				pause_length = 0;
				if(siren_count > 20) {
					if(zvei_ok()) {
						if(debugmodus > 2) {
							cout << endl;
							for(i=0; i<maxlength; i++) {
								cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
							}
							cout << " (Siren6)" << endl << endl;
						}
						std::string numString="";
						for(unsigned int counter=0;counter<maxlength;counter++) {
							// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
							numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
						}
						DisplayResult(numString,6,"Sirenenausloesung: Entwarnung");
					}
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					folge_position = 0;
					siren_count = 0;
				}
			}

			// got siren tone 12/16 - Probe
			if ( ((found_tones[0] == 12 && found_tones[1] == 16) || (found_tones[1] == 12 && found_tones[0] == 16)) && folge_position == maxlength) {
				siren_count++;
				pause_length = 0;
				if(siren_count > 20) {
					if(zvei_ok()) {
						if(debugmodus > 2) {
							cout << endl;
							for(i=0; i<maxlength; i++) {
								cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
							}
							cout << " (Siren3)" << endl << endl;
						}
						std::string numString="";
						for(unsigned int counter=0;counter<maxlength;counter++) {
							// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
							numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
						}
						DisplayResult(numString,3,"Sirenenausloesung: Probe");
					}
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					folge_position = 0;
					siren_count = 0;
				}
			}

			// got siren tone 12/17 - ZVS-Alarm
			if ( ((found_tones[0] == 12 && found_tones[1] == 17) || (found_tones[1] == 12 && found_tones[0] == 17)) && folge_position == maxlength) {
				siren_count++;
				pause_length = 0;
				if(siren_count > 20) {
					if(zvei_ok()) {
						if(debugmodus > 2) {
							cout << endl;
							for(i=0; i<maxlength; i++) {
								cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
							}
							cout << " (Siren4): " << found_tones[0] << " " << found_tones[1] << endl << endl;
						}
						std::string numString="";
						for(unsigned int counter=0;counter<maxlength;counter++) {
							// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
							numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
						}
						DisplayResult(numString,4,"Sirenenausloesung: ZVS-Alarm");
					}
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					folge_position = 0;
					siren_count = 0;
				}
			}

			// got siren tone 12/18 - ZVS-Warnung
			if ( ((found_tones[0] == 12 && found_tones[1] == 18) || (found_tones[1] == 12 && found_tones[0] == 18)) && folge_position == maxlength) {
				siren_count++;
				pause_length = 0;
				if(siren_count > 20) {
					if(zvei_ok()) {
						if(debugmodus > 2) {
							cout << endl;
							for(i=0; i<maxlength; i++) {
								cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
							}
							cout << " (Siren5)" << endl << endl;
						}
						std::string numString="";
						for(unsigned int counter=0;counter<maxlength;counter++) {
							// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
							numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
						}
						DisplayResult(numString,5,"Sirenenausloesung: ZVS-Warnung");
					}
					for(i=0; i<maxlength; i++) {
						zvei_folge[i] = -2;
					}
					folge_position = 0;
					siren_count = 0;
				}
			}

			// new digit after full zvei_folge
			if(((f >= 0 && f <= 11) || f == 14) && folge_position == maxlength && zvei_ok() && (zvei_folge[4] != f || pause_length > 10 )) {
				if(debugmodus > 2) {
					cout << endl;
					for(i=0; i<maxlength; i++) {
						cout << (zvei_folge[i] == 14 ? zvei_folge[(i+(maxlength-1))%maxlength] : zvei_folge[i]);
					}
					cout << " (Nachfolger): " << f << endl;
				}
				
				// Motorola Sonderbehandlung
				if(zvei_folge[0] == 14) {
					zvei_folge[0] = last_zvei_last_character;
				}
				
				std::string numString="";
				for(unsigned int counter=0;counter<maxlength;counter++) {
					// numString+=convertIntToHexString((zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter]));
					numString+=zvei_character[(zvei_folge[counter] == 14 ? zvei_folge[(counter+(maxlength-1))%maxlength] : zvei_folge[counter])];
				}
				DisplayResult(numString,0,"unklare Ausloesung");
				
				// first tone 14 not in ZVEI-Standard but used by Motorola radios when two select5 are sent directly following, this saves last character of decoded select5.
				if(zvei_folge[4] == 14) {
					last_zvei_last_character = zvei_folge[3];
				} else {
					last_zvei_last_character = zvei_folge[4];
				}

				for(i=0; i<maxlength; i++) {
					zvei_folge[i] = -2;
				}
				folge_position = 0;

				zvei_folge[folge_position] = f;
				folge_position++;

				pause_length = 0;
				siren_count = 0;
			}

			seven_count++;

			// just resetting our counter for the buffer of seven detected tones
			if(seven_count == 7) {
				seven_count = 0;
				if(false) { // DEBUG
					for(int i=0; i<7; i++) {
						cout << detected_seven[i][0] << " - " << detected_seven[i][1] << endl;
					}
					cout << endl;
				}
			}
		}
	}

}

/**
 * @brief fourier transformation (Goertzel) on given block of sound data
 */
int *MonitorModuleZVEI::process_block(int *found_tones)
{
	float tote = 0;
	float totte[2*sizeof(zvei_freq)/sizeof(zvei_freq[0])];
	unsigned int i, j;

	/*	Aufaddieren der energy-Eintraege nach TOTal-Energy (tote)	*/
	for (i = 0; i < BLOCKNUM; i++) tote += energy[i];

	/*	Aufaddieren der Zeilen der tenergy-Matrix nach TOTal-TEnergy (totte)	*/
	for (i = 0; i < 2*sizeof(zvei_freq)/sizeof(zvei_freq[0]); i++) {
		totte[i] = 0;
		for (j = 0; j < BLOCKNUM; j++)
			totte[i] += tenergy[j][i];
	}
	// tote *= (BLOCKNUM * BLOCKLEN * 0.5);  /* adjust for block lengths */
	/*	Summe der Quadrate der korrespondierenden Sinus- und Cosinuseintraege	*/
	for (i = 0; i < sizeof(zvei_freq)/sizeof(zvei_freq[0]); i++) totte[i] = fsqr(totte[i]) + fsqr(totte[i+sizeof(zvei_freq)/sizeof(zvei_freq[0])]);

	/*	Weiterschieben der energy-Eintraege, Ruecksetzen Feld [0] 	*/
	memmove(energy + 1, energy,
		sizeof(energy) - sizeof(energy[0]));
	energy[0] = 0;

	/*	Weiterschieben der tenergy-Spalten, Ruecksetzen Spalte [0] 	*/
	memmove(tenergy + 1, tenergy,
		sizeof(tenergy) - sizeof(tenergy[0]));
	memset(tenergy, 0, sizeof(tenergy[0]));

	tote = 0;
	for( i = 0; i < 2*sizeof(zvei_freq)/sizeof(zvei_freq[0]) ; i++ ) {
		tote += totte[i];
	}

	if(false) { // DEBUG
		printf("ZVEI: Energies: %8.5f\n0->%8.5f\t1->%8.5f\t2->%8.5f\n3->%8.5f\t4->%8.5f\t5->%8.5f\n6->%8.5f\t7->%8.5f\t"
		   "8->%8.5f\n9->%8.5f\t10(2800)->%8.5f\t11(810)->%8.5f\n12(675)->%8.5f\t13(1240)->%8.5f\t14(w)->%8.5f\n15(NULL)->%8.5f\t16(1860)->%8.5f\t17(825)->%8.5f\n18(2280)->%8.5f\t19(1010)->%8.5f\n\n",
		   tote, totte[0], totte[1], totte[2], totte[3], totte[4], totte[5], totte[6], totte[7],
		   totte[8], totte[9], totte[10], totte[11], totte[12], totte[13], totte[14], totte[15], totte[16], totte[17], totte[18], totte[19]);
	}

	if(debugmodus > 9) { //DEBUG, USE WITH CAUTION, generates possibly big logfile on long runtime!
		ofstream outfile;
		outfile.open ("freq_dmp.txt", ios_base::app);
		outfile << tote << "|";
		for(unsigned int count = 0; count < sizeof(zvei_freq)/sizeof(zvei_freq[0]); count++) {
			outfile << (int)totte[count] << "|";
		}
		outfile << endl;
		outfile.close();
	}

	// kein groesster index gefunden -> -1 zurueckgeben
	/*if ((i = find_max_index(totte, -1, -1)) < 0) {
		//return -1;
	} */

	found_tones[0] = find_max_index(totte, -1, -1); // groesste Energie
	found_tones[1] = find_max_index(totte, found_tones[0], -1); // zweitgroesste Energie
	found_tones[2] = find_max_index(totte, found_tones[0], found_tones[1]); // drittgroesste Energie
	found_tones[3] = (int) totte[found_tones[0]]; // Energie I
	found_tones[4] = (int) totte[found_tones[1]]; // Energie II
	found_tones[5] = (int) totte[found_tones[2]]; // Energie III
	found_tones[6] = (int) tote; // Total Energie

	if(false) { // DEBUG
		cout << "process_block/found_tones: " << endl;
		cout << found_tones[0] << " -> " << totte[found_tones[0]] << ", " << found_tones[1] << " -> " << totte[found_tones[1]] << ", " << found_tones[2] << " -> " << totte[found_tones[2]] << endl;
		cout << found_tones[3] << ", " << found_tones[4] << ", " << "Squelch: " << (squelch * found_tones[6]) << " (" << (found_tones[3] > (squelch * found_tones[6])) << ")" << endl << endl;
	}

	if(false) { // DEBUG
		cout << "Tone index: " << found_tones[0] << " above squelch: " << (found_tones[3] > (squelch * found_tones[6])) << endl;
	}

	// Energielevel passen nicht (hier ist der Energielevel des gefundenen Tons kleiner als 40% der Gesamtenergie, zu geringer Rauschabstand)
	//if ((tote * 0.4) > totte[index1]) return -1;

	return found_tones;
}

/**
 * @brief finds maximum energy from energies matrix given by MonitorModuleZVEI::process_block()
 * @param totte energies matrix created in MonitorModuleZVEI::process_block()
 * @return index pointing to frequency holding maximum energy/-1 if problem/filter
 */
int MonitorModuleZVEI::find_max_index(const float *totte, int index1, int index2)
{

	float en = 0;
	int index = -1;
	int i;

	/*	Ermitteln des Index' fuer den (erst-, zweit-, dritt-) groessten Eintrag	*/
	for (i = 0; i < (int)(sizeof(zvei_freq)/sizeof(zvei_freq[0])); i++) {
		if (totte[i] > en && i != index1 && i != index2){
			en = totte[i];
			index = i;
		}
	}

/*	en *= 0.25; // sirenenton ab 0.8 detektiert - Gruetze irgendwo/Algorithmus sinnvoll?

	for (i = 0; i < 16; i++) {
		if (index != i && totte[i] > en) return -1;
	} */

	return index;
}

/**
 * @brief finds a digit out of o block of seven buffers; must be same digit four times following for detection
 */
int MonitorModuleZVEI::fuzzyseven() {
	int tone0count = 0;
	int fail = 0;
	int tone0 = detected_seven[(seven_count + 1) % 7][0];

	if(debugmodus > 5) {
		cout << endl;
		for(int i=0; i<7; i++) {
				cout << detected_seven[(seven_count + i + 1) % 7][0] << " ";
		}
	}

	if(detected_seven[(seven_count + 1) % 7][0] == -1) {
		return -1; // deleted entry, already being detected
	}

	if(detected_seven[(seven_count + 1) % 7][3] < (squelch * detected_seven[(seven_count + 1) % 7][6])) {
		if(debugmodus > 3) {
			cout << "SQUELCH (" << detected_seven[(seven_count + 1) % 7][0] << ")!" << endl;
		}
		return -1; // Rauschsperre zu
	}

	for(int i = 0; i < 5; i++) {
		if(detected_seven[(seven_count + i + 1) % 7][0] == tone0) {
			tone0count++;
		} else {
			fail++;
		}
		if(debugmodus > 6) {
			cout << "tone0count: " << tone0count << " " << "(" << tone0 <<  ")";
		}
		if(tone0count == 4) {
			return tone0;
		}
	}
	return -1;
}

/**
 * @brief checks zvei tones for correctness
 */
bool MonitorModuleZVEI::zvei_ok() {

	// Geloeschte Folge oder Pause in der Tonfolge:
	for(unsigned int i = 0; i < maxlength; i++) {
		if(zvei_folge[i] == -1 || zvei_folge[i] == -2) {
			return false;
		}
	}

	// Fehlerfall: Doppeleintraege
	for(int i = 0; i < 4; i++) {
		if(zvei_folge[i] == zvei_folge[i+1]) {
			return false;
		}
	}

	// sonst: ok.
	return true;
}

/**
 * @brief output alarm/ZVEI-Tonfolge to connected clients (NO storing of data!)
 * @param Adresse ZVEI-Tonfolge
 * @param typ alarm type [0|1|2]
 * @param Text free text (human readable type)
 */
void MonitorModuleZVEI::DisplayResult(std::string Adresse,int typ, std::string Text)
{
	std::string alarmTypString ;
	std::string jetzt ;
	char dateStr[9];
	char timeStr[9];

	alarmTypString = convertIntToString(typ) ;

	// aktuelle Uhrzeit holen (Klartext & unix timestamp)
	currentTime(jetzt) ;
	struct tm* tm_time= localtime(&m_time) ;
	// FIXME warum nur ein zweistelliges Jahr?
	strftime(dateStr,9,"%d.%m.%y" ,tm_time) ;
	strftime(timeStr,9,"%H:%M:%S" ,tm_time) ;

	ModuleResultBase *pRes =new ModuleResultBase() ;

	pRes->set("timestamp",jetzt);
	pRes->set("uhrzeit",timeStr) ;
	pRes->set("datum",dateStr) ;
	pRes->set("servernamehex",m_serverNameHex);
	pRes->set("channelnamehex",m_channelNameHex);
	pRes->set("channelnum",convertIntToString(m_iChannelNum));

	pRes->set("typ","zvei");
	pRes->set("zvei",Adresse) ;
	pRes->set("weckton",alarmTypString) ;
	pRes->set("text",Text) ;

	FILE_LOG(logDEBUG) << endl << (*pRes) ;
	GlobalDispatcher->addResult(pRes) ;
}


/**
 * @brief output alarm/ZVEI-Tonfolge to some storage engine (NO displaying!)
 * @param Adresse ZVEI-Tonfolge
 * @param typ alarm type [0|1|2]
 * @param text free text (human readable type)
 */
void MonitorModuleZVEI::StoreResult(std::string Adresse,int typ,  std::string text)
{

}
