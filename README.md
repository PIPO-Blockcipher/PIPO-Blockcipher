# Blockcipher_PIPO
## PIPO_reference_bitslice.c
Bitslice version implementation code of block cipher PIPO.

## PIPO_reference_TLU.c
Table lookup version implementation code of block cipher PIPO.

## PIPO test vector
Plaintext: 0x098552F6_1E270026  
Secret key: 0x6DC416DD_779428D2_7E1D20AD_2E152297  
Ciphertext: 0x6B6B2981_AD5D0327

## PIPO_Analysis.exe
Differential and Linear trail searching program for PIPO.
### Options
-o [RotationOffset]  : Each Rotation Offset  
-c [D/L]             : Choose DC('D'), LC('L')  
-r [Round]           : Target round  
### Example
.\PIPO_Analysis.exe -o 07436512 -c D -r 7

## PIPO_Analysis
The source code of Differential and Linear trail searching program for PIPO.

For Windows 10 (or other Windows version),
   1) use Visual Studio to build.
   2) .\PIPO_Analysis.exe -o 07436512 -c L -r 7

For Ubuntu18.04 (or other Linux version),
   1) cd PIPO_Analysis
   2) make
   3) ./PIPO_Analysis.out -o 07436512 -c L -r 7

For the compilation, we used Visual Studio 2017 on Windows 10 or gcc 7.4.0 on Ubuntu18.04.
