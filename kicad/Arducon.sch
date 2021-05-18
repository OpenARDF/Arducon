EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr B 17000 11000
encoding utf-8
Sheet 1 2
Title "Arducon"
Date "2021-05-01"
Rev "P2.2"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GNDD #PWR0140
U 1 1 5797D116
P 4050 6400
F 0 "#PWR0140" H 4050 6150 50  0001 C CNN
F 1 "GNDD" H 4050 6250 50  0000 C CNN
F 2 "" H 4050 6400 50  0000 C CNN
F 3 "" H 4050 6400 50  0000 C CNN
	1    4050 6400
	0    1    1    0   
$EndComp
Text Label 5950 4200 0    39   ~ 0
PB0
Entry Wire Line
	6050 4200 6150 4300
Text Label 5950 4300 0    39   ~ 0
PB1
Text Label 5950 4400 0    39   ~ 0
PB2
Text Label 5950 4500 0    39   ~ 0
PB3
Text Label 5950 4600 0    39   ~ 0
PB4
Text Label 5950 4700 0    39   ~ 0
PB5
Text Label 5950 5050 0    39   ~ 0
PC0
Text Label 5950 5150 0    39   ~ 0
PC1
Text Label 5950 5250 0    39   ~ 0
PC2
Text Label 5950 5350 0    39   ~ 0
PC3
Text Label 5950 5450 0    39   ~ 0
SDA
Text Label 5950 5550 0    39   ~ 0
SCL
Text Label 5950 5650 0    39   ~ 0
~RESET~
Text Label 5950 5800 0    39   ~ 0
PD1
Text Label 5950 5900 0    39   ~ 0
PD0
Text Label 5950 6000 0    39   ~ 0
PD2
Text Label 5950 6100 0    39   ~ 0
PD3
Text Label 5950 6200 0    39   ~ 0
PD4
Text Label 5950 6300 0    39   ~ 0
PD5
Text Label 5950 6400 0    39   ~ 0
PD6
Text Label 5950 6500 0    39   ~ 0
PD7
Entry Wire Line
	6050 4300 6150 4400
Entry Wire Line
	6050 4400 6150 4500
Entry Wire Line
	6050 4500 6150 4600
Entry Wire Line
	6050 4600 6150 4700
Entry Wire Line
	6050 4700 6150 4800
Entry Wire Line
	6050 5050 6150 5150
Entry Wire Line
	6050 5150 6150 5250
Entry Wire Line
	6050 5250 6150 5350
Entry Wire Line
	6050 5350 6150 5450
Entry Wire Line
	6050 5450 6150 5550
Entry Wire Line
	6050 5550 6150 5650
Entry Wire Line
	6050 5650 6150 5750
Entry Wire Line
	6050 5800 6150 5900
Entry Wire Line
	6050 5900 6150 6000
Entry Wire Line
	6050 6000 6150 6100
Entry Wire Line
	6050 6100 6150 6200
Entry Wire Line
	6050 6200 6150 6300
Entry Wire Line
	6050 6300 6150 6400
Entry Wire Line
	6050 6400 6150 6500
Entry Wire Line
	6050 6500 6150 6600
Entry Wire Line
	6150 5050 6250 4950
Entry Wire Line
	6150 5150 6250 5050
Text Label 6250 4950 0    39   ~ 0
SCL
Text Label 6250 5050 0    39   ~ 0
SDA
Entry Wire Line
	6150 5500 6250 5400
Text Label 6250 5300 0    39   ~ 0
PC0
Entry Wire Line
	6150 5700 6250 5600
Text Label 6250 4200 0    39   ~ 0
PB0
Text Label 6250 4700 0    39   ~ 0
PB5
Text Label 4050 5550 2    39   ~ 0
ADC6
Text Label 4050 5650 2    39   ~ 0
ADC7
Entry Wire Line
	6150 4300 6250 4200
Text Label 6250 6400 0    39   ~ 0
PD6
Entry Wire Line
	6150 6600 6250 6500
Text Label 6250 6500 0    39   ~ 0
PD7
Entry Wire Line
	6150 6500 6250 6400
Text Label 6250 6200 0    39   ~ 0
PD4
Entry Wire Line
	6150 5900 6250 5800
Entry Wire Line
	6150 6000 6250 5900
Entry Wire Line
	6150 6100 6250 6000
Entry Wire Line
	6150 6200 6250 6100
Text Label 6250 5800 0    39   ~ 0
PD0
Text Label 6250 5500 0    39   ~ 0
PC2
Text Label 6250 6100 0    39   ~ 0
PD3
Text Label 6250 6000 0    39   ~ 0
PD2
Entry Wire Line
	6150 4400 6250 4300
Text Label 6250 4300 0    39   ~ 0
PB1
Entry Wire Line
	6150 5300 6250 5200
Text GLabel 7700 4950 2    39   Output ~ 0
SCL
Text GLabel 7700 5050 2    39   Output ~ 0
SDA
Entry Wire Line
	6150 6300 6250 6200
Text Label 6250 5900 0    39   ~ 0
PD1
Text Label 6250 5200 0    39   ~ 0
~RESET~
Text GLabel 6450 5200 2    39   Input ~ 0
~RESET~
$Comp
L Connector_Generic:Conn_02x03_Odd_Even P101
U 1 1 57AD5B1A
P 4900 7300
F 0 "P101" H 4900 7500 50  0000 C CNN
F 1 "ISP/PDI" H 4900 7100 50  0000 C CNN
F 2 "Pin_Headers:SM_Pin_Header_Straight_2x03" H 4900 6100 50  0001 C CNN
F 3 "http://katalog.we-online.de/em/datasheet/6100xx21121.pdf" H 4900 6100 50  0001 C CNN
F 4 "609-3487-1-ND" H 4900 7300 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=609-3487-1-ND\">Link</a>" H 4900 7300 60  0001 C CNN "Link"
F 6 "np" H 4900 7300 50  0001 C CNN "Manufacturer"
F 7 "np" H 4900 7300 50  0001 C CNN "Manufacturer PN"
	1    4900 7300
	-1   0    0    1   
$EndComp
Text GLabel 4500 7500 0    39   Output ~ 0
~RESET~
$Comp
L power:GNDD #PWR0145
U 1 1 57AD5D10
P 5200 7500
F 0 "#PWR0145" H 5200 7250 50  0001 C CNN
F 1 "GNDD" H 5200 7350 50  0000 C CNN
F 2 "" H 5200 7500 50  0000 C CNN
F 3 "" H 5200 7500 50  0000 C CNN
	1    5200 7500
	1    0    0    -1  
$EndComp
Text GLabel 5250 7300 2    39   Input ~ 0
MOSI
Entry Wire Line
	6150 4600 6250 4500
Text Label 6250 4500 0    39   ~ 0
PB3
Text GLabel 6750 4150 2    39   Output ~ 0
MOSI
Text GLabel 4450 7100 0    39   Input ~ 0
MISO
Entry Wire Line
	6150 4700 6250 4600
Text Label 6250 4600 0    39   ~ 0
PB4
Text GLabel 6750 4450 2    39   Output ~ 0
MISO
Entry Wire Line
	6150 4800 6250 4700
Text GLabel 6750 4750 2    39   Output ~ 0
SCK
Text GLabel 4450 7300 0    39   Input ~ 0
SCK
Entry Wire Line
	6150 6400 6250 6300
Text Label 6250 6300 0    39   ~ 0
PD5
Entry Wire Line
	6150 5600 6250 5500
Entry Wire Line
	6150 4500 6250 4400
Text Label 6250 5400 0    39   ~ 0
PC1
Text Label 6250 5600 0    39   ~ 0
PC3
Entry Wire Line
	6150 5400 6250 5300
Text Label 6250 4400 0    39   ~ 0
PB2
Text Notes 4300 3850 0    79   ~ 16
Microcontroller Module
$Comp
L power:GNDA #PWR0148
U 1 1 57B0FFE1
P 12100 10200
F 0 "#PWR0148" H 12100 9950 50  0001 C CNN
F 1 "GNDA" H 12100 10050 50  0000 C CNN
F 2 "" H 12100 10200 50  0000 C CNN
F 3 "" H 12100 10200 50  0000 C CNN
	1    12100 10200
	1    0    0    -1  
$EndComp
Text Label 5100 7300 0    39   ~ 0
PB3
Text Label 4600 7200 2    39   ~ 0
PB4
Text Label 4600 7300 2    39   ~ 0
PB5
Text Notes 6750 7550 0    79   ~ 16
               Real-Time Clock\n(Select only one of two device configurations)
Text Notes 12350 9500 0    118  ~ 24
PCB: Arducon ARDF Controller (p. 1/2)
$Comp
L power:GNDD #PWR0120
U 1 1 591E61CB
P 9250 9800
F 0 "#PWR0120" H 9250 9550 50  0001 C CNN
F 1 "GNDD" H 9250 9650 50  0000 C CNN
F 2 "" H 9250 9800 50  0000 C CNN
F 3 "" H 9250 9800 50  0000 C CNN
	1    9250 9800
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0116
U 1 1 591E902A
P 6400 8100
F 0 "#PWR0116" H 6400 7850 50  0001 C CNN
F 1 "GNDD" H 6400 7950 50  0000 C CNN
F 2 "" H 6400 8100 50  0000 C CNN
F 3 "" H 6400 8100 50  0000 C CNN
	1    6400 8100
	0    1    -1   0   
$EndComp
$Comp
L power:GNDD #PWR0109
U 1 1 591EABD4
P 9450 8550
F 0 "#PWR0109" H 9450 8300 50  0001 C CNN
F 1 "GNDD" H 9450 8400 50  0000 C CNN
F 2 "" H 9450 8550 50  0000 C CNN
F 3 "" H 9450 8550 50  0000 C CNN
	1    9450 8550
	0    -1   -1   0   
$EndComp
$Comp
L power:GNDD #PWR0147
U 1 1 59203F23
P 11850 10200
F 0 "#PWR0147" H 11850 9950 50  0001 C CNN
F 1 "GNDD" H 11850 10050 50  0000 C CNN
F 2 "" H 11850 10200 50  0000 C CNN
F 3 "" H 11850 10200 50  0000 C CNN
	1    11850 10200
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0127
U 1 1 591CAC09
P 3800 3700
F 0 "#PWR0127" H 3800 3550 50  0001 C CNN
F 1 "Vproc" H 3800 3850 50  0000 C CNN
F 2 "" H 3800 3700 50  0000 C CNN
F 3 "" H 3800 3700 50  0000 C CNN
	1    3800 3700
	1    0    0    -1  
$EndComp
Text GLabel 7900 9050 1    39   Output ~ 0
RTC_SQW
Text GLabel 6400 6000 2    39   Input ~ 0
RTC_SQW
$Comp
L Connector_Generic:Conn_01x01 W101
U 1 1 59FF6C44
P 12100 9750
F 0 "W101" H 12100 9850 50  0000 C CNN
F 1 "CONN_01X01" V 12200 9750 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 12100 9750 50  0001 C CNN
F 3 "" H 12100 9750 50  0001 C CNN
F 4 "np" H 12100 9750 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 12100 9750 60  0001 C CNN "Link"
	1    12100 9750
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 W102
U 1 1 59FF6D51
P 12100 9950
F 0 "W102" H 12100 10050 50  0000 C CNN
F 1 "CONN_01X01" V 12200 9950 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 12100 9950 50  0001 C CNN
F 3 "" H 12100 9950 50  0001 C CNN
F 4 "np" H 12100 9950 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 12100 9950 60  0001 C CNN "Link"
	1    12100 9950
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0135
U 1 1 5A0895E4
P 7400 4700
F 0 "#PWR0135" H 7400 4450 50  0001 C CNN
F 1 "GNDD" H 7550 4600 50  0000 C CNN
F 2 "" H 7400 4700 50  0000 C CNN
F 3 "" H 7400 4700 50  0000 C CNN
	1    7400 4700
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0107
U 1 1 5AB7DABB
P 9250 8150
F 0 "#PWR0107" H 9250 8000 50  0001 C CNN
F 1 "Vproc" H 9250 8300 50  0000 C CNN
F 2 "" H 9250 8150 50  0000 C CNN
F 3 "" H 9250 8150 50  0000 C CNN
	1    9250 8150
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:Battery_Single_Cell-Device BT101
U 1 1 5BCD438E
P 6550 8100
F 0 "BT101" V 6700 8000 50  0000 L CNN
F 1 "CR1220" V 6400 7900 50  0000 L CNN
F 2 "Oddities:BC501SM_CR1220_BAT_Handsolder" V 6550 8140 50  0001 C CNN
F 3 "" V 6550 8140 50  0000 C CNN
F 4 "BC501SM-ND" H 6550 8100 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=BC501SM-ND\">Link</a>" H 6550 8100 60  0001 C CNN "Link"
F 6 "Memory Protection Devices" V 6550 8100 50  0001 C CNN "Manufacturer"
F 7 "BC501SM" V 6550 8100 50  0001 C CNN "Manufacturer PN"
F 8 "CR1220 BATTERY HOLDER" V 6550 8100 50  0001 C CNN "Description"
	1    6550 8100
	0    1    -1   0   
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0144
U 1 1 5C60DF91
P 5200 7150
F 0 "#PWR0144" H 5200 7000 50  0001 C CNN
F 1 "Vproc" H 5200 7300 50  0000 C CNN
F 2 "" H 5200 7150 50  0000 C CNN
F 3 "" H 5200 7150 50  0000 C CNN
	1    5200 7150
	1    0    0    -1  
$EndComp
Text GLabel 8850 9050 0    39   Input ~ 0
SCL
Text GLabel 8850 9200 0    39   Input ~ 0
SDA
Text GLabel 6400 4200 2    39   Output ~ 0
D0
Text GLabel 6400 4300 2    39   Output ~ 0
D1
Text GLabel 6400 4400 2    39   Output ~ 0
D2
Text GLabel 6750 4250 2    39   Output ~ 0
D3
$Comp
L Device:R R103
U 1 1 5F9AFE5B
P 8600 8300
F 0 "R103" H 8700 8450 50  0000 C CNN
F 1 "10k" V 8500 8300 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 8530 8300 50  0001 C CNN
F 3 "" H 8600 8300 50  0001 C CNN
F 4 "311-10.0KCRCT-ND" H 8600 8300 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-10.0KCRCT-ND\">Link</a>" H 8600 8300 60  0001 C CNN "Link"
F 6 "Yageo" H 8600 8300 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-0710KL" H 8600 8300 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 10K OHM 1% 1/8W 0805" H 8600 8300 50  0001 C CNN "Description"
F 9 "C17414" H 8600 8300 50  0001 C CNN "LCSC Part Number"
	1    8600 8300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5950 4200 6050 4200
Wire Wire Line
	6050 4300 5950 4300
Wire Wire Line
	6050 4400 5950 4400
Wire Wire Line
	6050 4500 5950 4500
Wire Wire Line
	6050 4600 5950 4600
Wire Wire Line
	6050 4700 5950 4700
Wire Wire Line
	6050 5250 5950 5250
Wire Wire Line
	6050 5350 5950 5350
Wire Wire Line
	6050 5450 5950 5450
Wire Wire Line
	6050 5550 5950 5550
Wire Wire Line
	6050 5650 5950 5650
Wire Wire Line
	6050 5900 5950 5900
Wire Wire Line
	6050 6000 5950 6000
Wire Wire Line
	5950 6100 6050 6100
Wire Wire Line
	6050 6200 5950 6200
Wire Wire Line
	6050 6300 5950 6300
Wire Wire Line
	6050 6400 5950 6400
Wire Wire Line
	6050 6500 5950 6500
Wire Wire Line
	6400 4300 6250 4300
Wire Wire Line
	5100 7400 5200 7400
Wire Wire Line
	5200 7400 5200 7500
Wire Wire Line
	5100 7200 5200 7200
Wire Wire Line
	5200 7200 5200 7150
Wire Wire Line
	4600 7100 4600 7200
Wire Wire Line
	4450 7100 4600 7100
Wire Wire Line
	4600 7400 4600 7500
Wire Wire Line
	4600 7500 4500 7500
Wire Wire Line
	6050 5050 5950 5050
Wire Wire Line
	6050 5150 5950 5150
Wire Wire Line
	6050 5800 5950 5800
Wire Wire Line
	4450 7300 4600 7300
Wire Wire Line
	5250 7300 5100 7300
Wire Wire Line
	9250 8450 9250 8550
Connection ~ 9250 8550
Wire Wire Line
	6400 6000 6250 6000
Wire Wire Line
	6400 5300 6250 5300
Wire Wire Line
	11850 9650 11850 9750
Wire Wire Line
	3050 5650 3400 5650
Wire Wire Line
	6400 4200 6250 4200
Wire Wire Line
	6400 5400 6250 5400
Wire Wire Line
	6400 4400 6250 4400
Wire Wire Line
	6400 6400 6250 6400
Connection ~ 11850 10100
Wire Wire Line
	12100 10100 12100 10200
Wire Wire Line
	11900 9750 11850 9750
Wire Wire Line
	11600 9950 11850 9950
Connection ~ 11850 9950
Wire Wire Line
	7100 4950 7100 4700
Connection ~ 7100 4950
Wire Wire Line
	7200 5050 7200 4700
Connection ~ 7200 5050
Wire Wire Line
	6250 4950 7100 4950
Wire Wire Line
	6250 5050 7200 5050
Wire Wire Line
	6400 6500 6250 6500
Wire Wire Line
	8850 9550 8750 9550
Text GLabel 6400 6200 2    39   Output ~ 0
PTT_LOGIC
Text GLabel 6400 6300 2    39   Output ~ 0
TONE_LOGIC
Text GLabel 6400 6400 2    39   Output ~ 0
CW_KEY_LOGIC
Text GLabel 6400 6500 2    39   Output ~ 0
PWDN
Text GLabel 1500 4500 2    39   Input ~ 0
TONE_LOGIC
Text GLabel 4300 950  2    39   Input ~ 0
TONE_LOGIC
$Comp
L Device:R_POT RV101
U 1 1 5F9CC9A1
P 4150 1650
F 0 "RV101" H 3950 1550 50  0000 C CNN
F 1 "10k" H 4000 1650 50  0000 C CNN
F 2 "Potentiometers:Potentiometer_ST32ET_Handsolder" H 4150 1650 50  0001 C CNN
F 3 "" H 4150 1650 50  0001 C CNN
F 4 "TC33X-103ECT-ND" H 4150 1650 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=TC33X-103ECT-ND\">Link</a>" H 4150 1650 50  0001 C CNN "Link"
F 6 "Bourns Inc." H 4150 1650 50  0001 C CNN "Manufacturer"
F 7 "TC33X-2-103E" H 4150 1650 50  0001 C CNN "Manufacturer PN"
F 8 "TRIMMER 10K OHM 0.1W J LEAD TOP" H 4150 1650 50  0001 C CNN "Description"
F 9 "C719176" H 4150 1650 50  0001 C CNN "LCSC Part Number"
	1    4150 1650
	-1   0    0    1   
$EndComp
$Comp
L power:GNDD #PWR0110
U 1 1 5F9CD436
P 4150 1850
F 0 "#PWR0110" H 4150 1600 50  0001 C CNN
F 1 "GNDD" H 4150 1700 50  0000 C CNN
F 2 "" H 4150 1850 50  0000 C CNN
F 3 "" H 4150 1850 50  0000 C CNN
	1    4150 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 1050 4150 950 
Wire Wire Line
	4150 1350 4150 1500
Wire Wire Line
	4150 1800 4150 1850
Wire Wire Line
	4000 1650 3800 1650
Text Label 1650 2100 0    39   ~ 0
AUDIO_OUT
Text Label 1650 2900 0    39   ~ 0
AUDIO_IN
Text Label 1650 2500 0    39   ~ 0
PTT_OUT
Text Label 1650 2400 0    39   ~ 0
RADIO_V+
Text Label 1650 2200 0    39   ~ 0
GROUND
Wire Wire Line
	1650 2500 3250 2500
$Comp
L power:GNDD #PWR0115
U 1 1 5F9D0257
P 3850 3000
F 0 "#PWR0115" H 3850 2750 50  0001 C CNN
F 1 "GNDD" H 3850 2850 50  0000 C CNN
F 2 "" H 3850 3000 50  0000 C CNN
F 3 "" H 3850 3000 50  0000 C CNN
	1    3850 3000
	1    0    0    -1  
$EndComp
Text GLabel 4450 2800 2    39   Input ~ 0
PTT_LOGIC
$Comp
L power:GNDD #PWR0108
U 1 1 5F9DA49D
P 2100 2200
F 0 "#PWR0108" H 2100 1950 50  0001 C CNN
F 1 "GNDD" H 2100 2050 50  0001 C CNN
F 2 "" H 2100 2200 50  0000 C CNN
F 3 "" H 2100 2200 50  0000 C CNN
F 4 "Value" H 2100 2200 60  0001 C CNN "Part No."
F 5 "<a href=\"\">Link</a>" H 2100 2200 60  0001 C CNN "Link"
	1    2100 2200
	0    -1   -1   0   
$EndComp
$Comp
L Arducon-rescue:SPST_small-Switch SW101
U 1 1 5F9DCC49
P 7250 5600
F 0 "SW101" H 7250 5500 50  0000 C CNN
F 1 "SPST" H 7250 5500 50  0001 C CNN
F 2 "Buttons_Switches_SMD:PTS645SL50SMTR92" H 7250 5600 50  0001 C CNN
F 3 "" H 7250 5600 50  0001 C CNN
F 4 "CKN9088CT-ND" H 7250 5600 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CKN9088CT-ND\">Link</a>" H 7250 5600 50  0001 C CNN "Link"
F 6 "C&K" H 7250 5600 50  0001 C CNN "Manufacturer"
F 7 "PTS645SL50SMTR92LFS" H 7250 5600 50  0001 C CNN "Manufacturer PN"
F 8 "SWITCH TACTILE SPST-NO 0.05A 12V" H 7250 5600 50  0001 C CNN "Description"
F 9 "C221877" H 7250 5600 50  0001 C CNN "LCSC Part Number"
	1    7250 5600
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0139
U 1 1 5F9DCF27
P 7500 5600
F 0 "#PWR0139" H 7500 5350 50  0001 C CNN
F 1 "GNDD" H 7500 5450 50  0000 C CNN
F 2 "" H 7500 5600 50  0000 C CNN
F 3 "" H 7500 5600 50  0000 C CNN
	1    7500 5600
	0    -1   -1   0   
$EndComp
Text GLabel 1500 4700 2    39   Input ~ 0
PTT_LOGIC
Text GLabel 14750 8300 0    39   Output ~ 0
EXT_PWR_GROUND
$Comp
L power:GNDD #PWR0124
U 1 1 5F9DFFB0
P 1500 4400
F 0 "#PWR0124" H 1500 4150 50  0001 C CNN
F 1 "GNDD" H 1500 4250 50  0001 C CNN
F 2 "" H 1500 4400 50  0000 C CNN
F 3 "" H 1500 4400 50  0000 C CNN
F 4 "Value" H 1500 4400 60  0001 C CNN "Part No."
F 5 "<a href=\"\">Link</a>" H 1500 4400 60  0001 C CNN "Link"
	1    1500 4400
	0    -1   -1   0   
$EndComp
Text GLabel 2750 2150 2    39   Output ~ 0
EXT_BATTERY
Text GLabel 11600 9950 0    39   Input ~ 0
EXT_PWR_GROUND
$Comp
L Device:R R107
U 1 1 5F9E5B53
P 2750 5650
F 0 "R107" V 2830 5650 50  0000 C CNN
F 1 "162k" V 2650 5650 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2680 5650 50  0001 C CNN
F 3 "" H 2750 5650 50  0001 C CNN
F 4 "311-162KCRCT-ND" H 2750 5650 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-162KCRCT-ND\">Link</a>" H 2750 5650 50  0001 C CNN "Link"
F 6 "Yageo" V 2750 5650 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-07162KL" V 2750 5650 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 162K OHM 1% 1/8W 0805" V 2750 5650 50  0001 C CNN "Description"
F 9 "C416820" V 2750 5650 50  0001 C CNN "LCSC Part Number"
	1    2750 5650
	0    1    1    0   
$EndComp
$Comp
L power:GNDA #PWR0137
U 1 1 5F9E60D1
P 3250 6150
F 0 "#PWR0137" H 3250 5900 50  0001 C CNN
F 1 "GNDA" H 3250 6000 50  0000 C CNN
F 2 "" H 3250 6150 50  0000 C CNN
F 3 "" H 3250 6150 50  0000 C CNN
	1    3250 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 5750 3050 5650
Connection ~ 3400 5650
Wire Wire Line
	3400 6100 3400 6000
Wire Wire Line
	3050 6100 3250 6100
Wire Wire Line
	3050 6100 3050 5950
Wire Wire Line
	3250 6150 3250 6100
Connection ~ 3250 6100
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5F9E89FA
P 11850 9650
F 0 "#FLG0102" H 11850 9745 50  0001 C CNN
F 1 "PWR_FLAG" H 11850 9830 50  0000 C CNN
F 2 "" H 11850 9650 50  0000 C CNN
F 3 "" H 11850 9650 50  0000 C CNN
	1    11850 9650
	1    0    0    -1  
$EndComp
Connection ~ 11850 9750
Wire Wire Line
	9250 8550 9250 8800
Wire Wire Line
	11850 10100 11850 10200
Wire Wire Line
	11850 10100 12100 10100
Wire Wire Line
	11850 9950 11850 10100
Wire Wire Line
	11850 9950 11900 9950
Wire Wire Line
	3250 2500 3850 2500
Wire Wire Line
	3400 5650 4050 5650
Wire Wire Line
	3400 5650 3400 5700
Wire Wire Line
	3250 6100 3400 6100
Wire Wire Line
	11850 9750 11850 9950
$Comp
L Device:R R101
U 1 1 5FB82EE5
P 4150 1200
F 0 "R101" V 4250 1200 50  0000 C CNN
F 1 "100k" V 4050 1200 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 4080 1200 50  0001 C CNN
F 3 "" H 4150 1200 50  0001 C CNN
F 4 "311-100KCRCT-ND" H 4150 1200 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-100KCRCT-ND\">Link</a>" H 4150 1200 60  0001 C CNN "Link"
F 6 "Yageo" V 4150 1200 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-07100KL" V 4150 1200 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 100K OHM 1% 1/8W 0805" V 4150 1200 50  0001 C CNN "Description"
F 9 "C96346" V 4150 1200 50  0001 C CNN "LCSC Part Number"
	1    4150 1200
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:Arduino_Pro_Mini-MCU_Module-MCU_Module MOD101
U 1 1 5FC82DF4
P 4950 5300
F 0 "MOD101" H 5000 6667 61  0000 C CNB
F 1 "Arduino Pro Mini (5V Version)" H 5000 6576 61  0000 C CNB
F 2 "Modules:Arduino_Pro_Mini" H 5000 6575 50  0001 C CIN
F 3 "" H 4950 5300 50  0001 C CNN
F 4 "1568-1055-ND" H 4950 5300 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=1568-1055-ND\">Link</a>" H 4950 5300 50  0001 C CNN "Link"
F 6 "SparkFun Electronics" H 4950 5300 50  0001 C CNN "Manufacturer"
F 7 "DEV-11113" H 4950 5300 50  0001 C CNN "Manufacturer PN"
F 8 "ARDUINO PRO MINI 328 5V/16MHZ" H 4950 5300 50  0001 C CNN "Description"
	1    4950 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 5050 7700 5050
Wire Wire Line
	7100 4950 7700 4950
NoConn ~ 4050 4600
NoConn ~ 4050 4800
NoConn ~ 4050 4700
Wire Wire Line
	6450 5200 6250 5200
Text GLabel 6750 4550 2    39   Output ~ 0
D4
Text GLabel 6750 4850 2    39   Output ~ 0
D5
$Comp
L Device:C_Small C108
U 1 1 5FF2B1B5
P 2600 4150
F 0 "C108" V 2500 4150 50  0000 L CNN
F 1 "100nF" V 2700 4050 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 2600 4150 50  0001 C CNN
F 3 "" H 2600 4150 50  0001 C CNN
F 4 "478-1395-1-ND" H 2600 4150 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=478-1395-1-ND\">Link</a>" H 2600 4150 60  0001 C CNN "Link"
F 6 "AVX Corporation" V 2600 4150 50  0001 C CNN "Manufacturer"
F 7 "08055C104KAT2A" V 2600 4150 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 0.1UF 50V X7R 0805" V 2600 4150 50  0001 C CNN "Description"
F 9 "C476766" V 2600 4150 50  0001 C CNN "LCSC Part Number"
	1    2600 4150
	0    -1   1    0   
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0134
U 1 1 5FF2E22F
P 2950 3550
F 0 "#PWR0134" H 2950 3400 50  0001 C CNN
F 1 "Vproc" H 2950 3700 50  0000 C CNN
F 2 "" H 2950 3550 50  0000 C CNN
F 3 "" H 2950 3550 50  0000 C CNN
	1    2950 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 4100 2950 4150
Wire Wire Line
	2700 4150 2950 4150
Connection ~ 2950 4150
Wire Wire Line
	2950 4150 2950 4200
NoConn ~ 4050 6100
$Comp
L power:GNDD #PWR0138
U 1 1 5FF4219C
P 4050 6200
F 0 "#PWR0138" H 4050 5950 50  0001 C CNN
F 1 "GNDD" H 4050 6050 50  0000 C CNN
F 2 "" H 4050 6200 50  0000 C CNN
F 3 "" H 4050 6200 50  0000 C CNN
	1    4050 6200
	0    1    1    0   
$EndComp
Wire Wire Line
	2950 4150 3550 4150
Text GLabel 3400 6500 0    39   Input ~ 0
EXT_BATTERY
Wire Wire Line
	3700 6500 3750 6500
$Comp
L Arducon-rescue:F1975NCGI-RF U104
U 1 1 5FF7DEC4
P 14100 2800
F 0 "U104" H 14350 3450 61  0000 L CNB
F 1 "F1975NCGI" H 14350 3300 61  0000 L CNB
F 2 "Housings_DFN_QFN:QFN-20-1EP_4x4mm_Pitch0.5mm" H 14200 3550 50  0001 C CNN
F 3 "" H 14170 2705 50  0001 C CNN
F 4 "800-3576-ND" H 14100 2800 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=800-3576-ND\">Link</a>" H 14100 2800 50  0001 C CNN "Link"
F 6 "Renesas Electronics America Inc" H 14100 2800 50  0001 C CNN "Manufacturer"
F 7 "F1975NCGI" H 14100 2800 50  0001 C CNN "Manufacturer PN"
F 8 "RF ATTENUATOR 31DB 75OHM 20WFQFN" H 14100 2800 50  0001 C CNN "Description"
F 9 "C1525854" H 14100 2800 50  0001 C CNN "LCSC Part Number"
	1    14100 2800
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:CD74HC4050M96-74xx-Logic_LevelTranslator U102
U 1 1 5FFA3B6A
P 10100 5250
F 0 "U102" H 10250 5400 50  0000 C CNB
F 1 "CD74HC4050M96" H 10075 5476 50  0001 C CNB
F 2 "Housings_SOIC:SOIC-16_3.9x9.9mm_Pitch1.27mm" H 10350 4600 50  0001 C CNN
F 3 "" H 10100 5250 50  0001 C CNN
F 4 "296-14529-1-ND" H 10100 5250 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=296-14529-1-ND\">Link</a>" H 10100 5250 50  0001 C CNN "Link"
	1    10100 5250
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:CD74HC4050M96-74xx-Logic_LevelTranslator U102
U 2 1 5FFA7F9B
P 10100 4700
F 0 "U102" H 10200 4850 50  0000 C CNB
F 1 "CD74HC4050M96" H 10100 4926 50  0001 C CNB
F 2 "Housings_SOIC:SOIC-16_3.9x9.9mm_Pitch1.27mm" H 10350 4050 50  0001 C CNN
F 3 "" H 10100 4700 50  0001 C CNN
F 4 "296-14529-1-ND" H 10100 4700 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=296-14529-1-ND\">Link</a>" H 10100 4700 50  0001 C CNN "Link"
	2    10100 4700
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:CD74HC4050M96-74xx-Logic_LevelTranslator U102
U 3 1 5FFA8DF3
P 10100 4150
F 0 "U102" H 10200 4300 50  0000 C CNB
F 1 "CD74HC4050M96" H 10100 4376 50  0001 C CNB
F 2 "Housings_SOIC:SOIC-16_3.9x9.9mm_Pitch1.27mm" H 10350 3500 50  0001 C CNN
F 3 "" H 10100 4150 50  0001 C CNN
F 4 "296-14529-1-ND" H 10100 4150 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=296-14529-1-ND\">Link</a>" H 10100 4150 50  0001 C CNN "Link"
	3    10100 4150
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:CD74HC4050M96-74xx-Logic_LevelTranslator U102
U 4 1 5FFA9DCC
P 10100 3700
F 0 "U102" H 10200 3850 50  0000 C CNB
F 1 "CD74HC4050M96" H 10100 3926 50  0001 C CNB
F 2 "Housings_SOIC:SOIC-16_3.9x9.9mm_Pitch1.27mm" H 10350 3050 50  0001 C CNN
F 3 "" H 10100 3700 50  0001 C CNN
F 4 "296-14529-1-ND" H 10100 3700 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=296-14529-1-ND\">Link</a>" H 10100 3700 50  0001 C CNN "Link"
F 6 "Texas Instruments" H 10100 3700 50  0001 C CNN "Manufacturer"
F 7 "CD74HC4050M96" H 10100 3700 50  0001 C CNN "Manufacturer PN"
F 8 "IC BUFFER NON-INVERT 6V 16SOIC" H 10100 3700 50  0001 C CNN "Description"
F 9 "C131932" H 10100 3700 50  0001 C CNN "LCSC Part Number"
	4    10100 3700
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:CD74HC4050M96-74xx-Logic_LevelTranslator U102
U 5 1 5FFAB336
P 10100 3200
F 0 "U102" H 10200 3350 50  0000 C CNB
F 1 "CD74HC4050M96" H 10100 3426 50  0001 C CNB
F 2 "Housings_SOIC:SOIC-16_3.9x9.9mm_Pitch1.27mm" H 10350 2550 50  0001 C CNN
F 3 "" H 10100 3200 50  0001 C CNN
F 4 "296-14529-1-ND" H 10100 3200 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=296-14529-1-ND\">Link</a>" H 10100 3200 50  0001 C CNN "Link"
	5    10100 3200
	1    0    0    -1  
$EndComp
$Comp
L Arducon-rescue:CD74HC4050M96-74xx-Logic_LevelTranslator U102
U 6 1 5FFAC3C8
P 10100 2700
F 0 "U102" H 10250 2850 50  0000 C CNB
F 1 "CD74HC4050M96" H 10050 3100 61  0000 C CNB
F 2 "Housings_SOIC:SOIC-16_3.9x9.9mm_Pitch1.27mm" H 10350 2050 50  0001 C CNN
F 3 "" H 10100 2700 50  0001 C CNN
F 4 " 296-14529-1-ND" H 10100 2700 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=296-14529-1-ND\">Link</a>" H 10100 2700 50  0001 C CNN "Link"
	6    10100 2700
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0133
U 1 1 5FFBCDD7
P 10100 5350
F 0 "#PWR0133" H 10100 5100 50  0001 C CNN
F 1 "GNDD" H 10300 5300 50  0000 C CNN
F 2 "" H 10100 5350 50  0000 C CNN
F 3 "" H 10100 5350 50  0000 C CNN
	1    10100 5350
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Linear:MIC5219-3.3YM5 U101
U 1 1 5FFBE552
P 10350 1700
F 0 "U101" H 10750 1600 61  0000 C CNB
F 1 "LP2992AIM5-3.3" H 11000 1450 61  0000 C CNB
F 2 "TO_SOT_Packages_SMD:SOT-23-5" H 10350 2025 50  0001 C CNN
F 3 "" H 10350 1700 50  0001 C CNN
F 4 "LP2992AIM5-3.3/NOPBCT-ND" H 10350 1700 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=LP2992AIM5-3.3/NOPBCT-ND\">Link</a>" H 10350 1700 50  0001 C CNN "Link"
F 6 "Texas Instruments" H 10350 1700 50  0001 C CNN "Manufacturer"
F 7 "LP2992AIM5-3.3/NOPB" H 10350 1700 50  0001 C CNN "Manufacturer PN"
F 8 "IC REG LINEAR 3.3V 250MA SOT23-5" H 10350 1700 50  0001 C CNN "Description"
F 9 "C37970" H 10350 1700 50  0001 C CNN "LCSC Part Number"
	1    10350 1700
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0105
U 1 1 5FFC0305
P 10350 2000
F 0 "#PWR0105" H 10350 1750 50  0001 C CNN
F 1 "GNDD" H 10350 1850 50  0000 C CNN
F 2 "" H 10350 2000 50  0000 C CNN
F 3 "" H 10350 2000 50  0000 C CNN
	1    10350 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C101
U 1 1 5FFC23DD
P 9500 1600
F 0 "C101" V 9600 1500 50  0000 L CNN
F 1 "10uF" V 9400 1500 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 9500 1600 50  0001 C CNN
F 3 "http://www.kemet.com/Lists/ProductCatalog/Attachments/53/KEM_C1002_X7R_SMD.pdf" H 9500 1600 50  0001 C CNN
F 4 "490-5523-1-ND" H 9500 1600 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=490-5523-1-ND\">Link</a>" H 9500 1600 60  0001 C CNN "Link"
F 6 "Murata Electronics" V 9500 1600 50  0001 C CNN "Manufacturer"
F 7 "GRM21BR61E106KA73L" V 9500 1600 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 10UF 25V X5R 0805" V 9500 1600 50  0001 C CNN "Description"
F 9 "C15850" V 9500 1600 50  0001 C CNN "LCSC Part Number"
	1    9500 1600
	0    -1   -1   0   
$EndComp
$Comp
L power:GNDD #PWR0103
U 1 1 5FFC45A0
P 9400 1600
F 0 "#PWR0103" H 9400 1350 50  0001 C CNN
F 1 "GNDD" H 9400 1450 50  0000 C CNN
F 2 "" H 9400 1600 50  0000 C CNN
F 3 "" H 9400 1600 50  0000 C CNN
	1    9400 1600
	0    1    1    0   
$EndComp
Wire Wire Line
	10050 1600 9950 1600
Wire Wire Line
	9750 1450 9750 1600
Connection ~ 9750 1600
Wire Wire Line
	9750 1600 9600 1600
NoConn ~ 10650 1700
$Comp
L Device:C_Small C102
U 1 1 5FFD9ABC
P 11150 1600
F 0 "C102" V 11250 1500 50  0000 L CNN
F 1 "10uF" V 11050 1500 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 11150 1600 50  0001 C CNN
F 3 "http://www.kemet.com/Lists/ProductCatalog/Attachments/53/KEM_C1002_X7R_SMD.pdf" H 11150 1600 50  0001 C CNN
F 4 "490-5523-1-ND" H 11150 1600 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=490-5523-1-ND\">Link</a>" H 11150 1600 60  0001 C CNN "Link"
F 6 "Murata Electronics" V 11150 1600 50  0001 C CNN "Manufacturer"
F 7 "GRM21BR61E106KA73L" V 11150 1600 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 10UF 25V X5R 0805" V 11150 1600 50  0001 C CNN "Description"
F 9 "C15850" V 11150 1600 50  0001 C CNN "LCSC Part Number"
	1    11150 1600
	0    1    -1   0   
$EndComp
$Comp
L power:GNDD #PWR0104
U 1 1 5FFDB579
P 11250 1600
F 0 "#PWR0104" H 11250 1350 50  0001 C CNN
F 1 "GNDD" H 11250 1450 50  0000 C CNN
F 2 "" H 11250 1600 50  0000 C CNN
F 3 "" H 11250 1600 50  0000 C CNN
	1    11250 1600
	0    -1   1    0   
$EndComp
Wire Wire Line
	11050 1600 10900 1600
$Comp
L power:+3.3V #PWR0102
U 1 1 5FFE2203
P 10900 1450
F 0 "#PWR0102" H 10900 1300 50  0001 C CNN
F 1 "+3.3V" H 10915 1623 50  0000 C CNN
F 2 "" H 10900 1450 50  0001 C CNN
F 3 "" H 10900 1450 50  0001 C CNN
	1    10900 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	10900 1450 10900 1600
Connection ~ 10900 1600
Wire Wire Line
	10900 1600 10700 1600
$Comp
L power:+3.3V #PWR0106
U 1 1 5FFE84F8
P 10100 2600
F 0 "#PWR0106" H 10100 2450 50  0001 C CNN
F 1 "+3.3V" H 10115 2773 50  0000 C CNN
F 2 "" H 10100 2600 50  0001 C CNN
F 3 "" H 10100 2600 50  0001 C CNN
	1    10100 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	10400 2700 10500 2700
Wire Wire Line
	10900 4100 11450 4100
Wire Wire Line
	10400 3200 10500 3200
Wire Wire Line
	10800 3200 10800 4200
Wire Wire Line
	10800 4200 11450 4200
Wire Wire Line
	10400 4700 10500 4700
Wire Wire Line
	10850 4700 10850 4500
Wire Wire Line
	10850 4500 11450 4500
Wire Wire Line
	10950 5250 10950 4600
Wire Wire Line
	10950 4600 11450 4600
Text GLabel 9000 4550 0    39   Input ~ 0
D0
Wire Wire Line
	10400 5250 10500 5250
Text GLabel 9000 4450 0    39   Input ~ 0
D1
Text GLabel 9000 4350 0    39   Input ~ 0
D2
Text GLabel 9000 4250 0    39   Input ~ 0
D3
Text GLabel 9000 4150 0    39   Input ~ 0
D4
Text GLabel 9000 4050 0    39   Input ~ 0
D5
NoConn ~ 14200 2350
NoConn ~ 14000 2350
$Comp
L Connector:DB9_Female J102
U 1 1 60066707
P 1350 2500
F 0 "J102" H 1550 2950 50  0000 C CNN
F 1 "DB9 Female" V 1600 2550 50  0000 C CNN
F 2 "Connect:DB9F_CI" H 1350 2500 50  0001 C CNN
F 3 "" H 1350 2500 50  0001 C CNN
F 4 "609-1525-ND" H 1350 2500 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=609-1525-ND\">Link</a>" H 1350 2500 50  0001 C CNN "Link"
F 6 "TH" H 1350 2500 50  0001 C CNN "Manufacturer"
F 7 "TH" H 1350 2500 50  0001 C CNN "Manufacturer PN"
	1    1350 2500
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_Coaxial J103
U 1 1 60068C60
P 15100 2550
F 0 "J103" H 15200 2525 50  0000 L CNN
F 1 "To Antenna" H 15200 2434 50  0000 L CNN
F 2 "Connect:CON-SMA-EDGE-S" H 15100 2550 50  0001 C CNN
F 3 "" H 15100 2550 50  0001 C CNN
F 4 "CON-SMA-EDGE-S-ND" H 15100 2550 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CON-SMA-EDGE-S-ND\">Link</a>" H 15100 2550 50  0001 C CNN "Link"
F 6 "TH" H 15100 2550 50  0001 C CNN "Manufacturer"
F 7 "TH" H 15100 2550 50  0001 C CNN "Manufacturer PN"
F 8 "TH" H 15100 2550 50  0001 C CNN "Description"
	1    15100 2550
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_Coaxial J101
U 1 1 6006B6D9
P 13200 2550
F 0 "J101" H 13300 2525 50  0000 L CNN
F 1 "To Radio" H 13300 2434 50  0000 L CNN
F 2 "Connect:CON-SMA-EDGE-S" H 13200 2550 50  0001 C CNN
F 3 "" H 13200 2550 50  0001 C CNN
F 4 "CON-SMA-EDGE-S-ND" H 13200 2550 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CON-SMA-EDGE-S-ND\">Link</a>" H 13200 2550 50  0001 C CNN "Link"
F 6 "TH" H 13200 2550 50  0001 C CNN "Manufacturer"
F 7 "TH" H 13200 2550 50  0001 C CNN "Manufacturer PN"
F 8 "TH" H 13200 2550 50  0001 C CNN "Description"
	1    13200 2550
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3850 2600 3850 2500
$Comp
L Arducon-rescue:LMN200B01-Power_Protection Q102
U 1 1 5FC59E4B
P 13550 7600
F 0 "Q102" H 13550 8200 61  0000 C CNB
F 1 "LMN200" H 13550 8092 61  0000 C CNB
F 2 "TO_SOT_Packages_SMD:SOT-363-0.65" H 13565 7400 50  0001 L CNN
F 3 "" H 13565 7400 50  0001 L CNN
F 4 "LMN200B02DICT-ND" H 13550 7600 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=LMN200B02DICT-ND\">Link</a>" H 13550 7600 50  0001 C CNN "Link"
F 6 "Diodes Incorporated" H 13550 7600 50  0001 C CNN "Manufacturer"
F 7 "LMN200B02-7" H 13550 7600 50  0001 C CNN "Manufacturer PN"
F 8 "MCU LOAD SWITCH 200MA SOT-363" H 13550 7600 50  0001 C CNN "Description"
F 9 "C150757" H 13550 7600 50  0001 C CNN "LCSC Part Number"
	1    13550 7600
	1    0    0    -1  
$EndComp
Text GLabel 13300 7400 0    39   Input ~ 0
PWDN
$Comp
L power:GNDD #PWR0141
U 1 1 5FC5EC97
P 13800 7500
F 0 "#PWR0141" H 13800 7250 50  0001 C CNN
F 1 "GNDD" H 13800 7350 50  0000 C CNN
F 2 "" H 13800 7500 50  0000 C CNN
F 3 "" H 13800 7500 50  0000 C CNN
	1    13800 7500
	0    -1   -1   0   
$EndComp
Wire Wire Line
	13800 7250 13900 7250
Wire Wire Line
	13900 7250 13900 6850
Wire Wire Line
	13900 6850 12900 6850
Wire Wire Line
	12900 6850 12900 7800
Wire Wire Line
	12900 7800 13300 7800
Wire Wire Line
	13800 7700 14200 7700
Text Notes 13150 6650 0    79   ~ 16
OPTIONAL EXTERNAL POWER CONTROL
$Comp
L Connector_Generic:Conn_01x02 J108
U 1 1 5FCE82B0
P 14950 7800
F 0 "J108" H 15030 7792 50  0000 L CNN
F 1 "Controlled Power" H 15100 7700 50  0000 L CNN
F 2 "Wire_Pads:SolderWirePad_2x_1-5mmDrill_Polarized" H 14950 7800 50  0001 C CNN
F 3 "" H 14950 7800 50  0001 C CNN
F 4 "np" H 14950 7800 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 14950 7800 50  0001 C CNN "Link"
	1    14950 7800
	1    0    0    1   
$EndComp
Wire Wire Line
	3800 4200 4050 4200
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5FD3014C
P 3750 6500
F 0 "#FLG0101" H 3750 6575 50  0001 C CNN
F 1 "PWR_FLAG" H 3750 6673 50  0001 C CNN
F 2 "" H 3750 6500 50  0001 C CNN
F 3 "~" H 3750 6500 50  0001 C CNN
	1    3750 6500
	1    0    0    -1  
$EndComp
Connection ~ 3750 6500
$Comp
L power:GNDA #PWR0136
U 1 1 5FD4CF42
P 2950 4500
F 0 "#PWR0136" H 2950 4250 50  0001 C CNN
F 1 "GNDA" H 2950 4350 50  0000 C CNN
F 2 "" H 2950 4500 50  0000 C CNN
F 3 "" H 2950 4500 50  0000 C CNN
	1    2950 4500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R105
U 1 1 5FDA4176
P 2950 3950
F 0 "R105" V 3050 3950 50  0000 C CNN
F 1 "1M" V 2850 3950 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2880 3950 50  0001 C CNN
F 3 "" H 2950 3950 50  0001 C CNN
F 4 "RMCF0805FT1M00CT-ND" H 2950 3950 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=RMCF0805FT1M00CT-ND\">Link</a>" H 2950 3950 60  0001 C CNN "Link"
F 6 "Stackpole Electronics Inc" V 2950 3950 50  0001 C CNN "Manufacturer"
F 7 "RMCF0805FT1M00" V 2950 3950 50  0001 C CNN "Manufacturer PN"
F 8 "RES 1M OHM 1% 1/8W 0805" V 2950 3950 50  0001 C CNN "Description"
F 9 "C107700" V 2950 3950 50  0001 C CNN "LCSC Part Number"
	1    2950 3950
	1    0    0    -1  
$EndComp
$Comp
L Device:R R106
U 1 1 5FDB4B7E
P 2950 4350
F 0 "R106" V 3050 4350 50  0000 C CNN
F 1 "100k" V 2850 4350 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2880 4350 50  0001 C CNN
F 3 "" H 2950 4350 50  0001 C CNN
F 4 "311-100KCRCT-ND" H 2950 4350 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-100KCRCT-ND\">Link</a>" H 2950 4350 60  0001 C CNN "Link"
F 6 "Yageo" V 2950 4350 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-07100KL" V 2950 4350 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 100K OHM 1% 1/8W 0805" V 2950 4350 50  0001 C CNN "Description"
F 9 "C96346" V 2950 4350 50  0001 C CNN "LCSC Part Number"
	1    2950 4350
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C103
U 1 1 5FCC669B
P 3700 1650
F 0 "C103" V 3600 1650 50  0000 L CNN
F 1 "100nF" V 3800 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 3700 1650 50  0001 C CNN
F 3 "" H 3700 1650 50  0001 C CNN
F 4 "478-1395-1-ND" H 3700 1650 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=478-1395-1-ND\">Link</a>" H 3700 1650 60  0001 C CNN "Link"
F 6 "AVX Corporation" V 3700 1650 50  0001 C CNN "Manufacturer"
F 7 "08055C104KAT2A" V 3700 1650 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 0.1UF 50V X7R 0805" V 3700 1650 50  0001 C CNN "Description"
F 9 "C476766" V 3700 1650 50  0001 C CNN "LCSC Part Number"
	1    3700 1650
	0    -1   1    0   
$EndComp
$Comp
L Device:C_Small C109
U 1 1 5FCD929C
P 3050 5850
F 0 "C109" V 2950 5850 50  0000 L CNN
F 1 "100nF" V 3150 5750 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 3050 5850 50  0001 C CNN
F 3 "" H 3050 5850 50  0001 C CNN
F 4 "478-1395-1-ND" H 3050 5850 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=478-1395-1-ND\">Link</a>" H 3050 5850 60  0001 C CNN "Link"
F 6 "AVX Corporation" V 3050 5850 50  0001 C CNN "Manufacturer"
F 7 "08055C104KAT2A" V 3050 5850 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 0.1UF 50V X7R 0805" V 3050 5850 50  0001 C CNN "Description"
F 9 "C476766" V 3050 5850 50  0001 C CNN "LCSC Part Number"
	1    3050 5850
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9300 2700 9650 2700
Wire Wire Line
	9400 4150 9400 3200
Wire Wire Line
	9400 3200 9650 3200
Wire Wire Line
	9300 4550 9300 5250
Wire Wire Line
	9300 5250 9650 5250
Wire Wire Line
	9400 4450 9400 4700
Wire Wire Line
	9400 4700 9650 4700
Wire Wire Line
	9300 2700 9300 4050
Wire Wire Line
	10900 2700 10900 4100
$Comp
L Jumper:SolderJumper_2_Open SJ103
U 1 1 5FDC8A28
P 10100 2900
F 0 "SJ103" H 10250 3000 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10100 3014 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10100 2900 50  0001 C CNN
F 3 "" H 10100 2900 50  0001 C CNN
F 4 "np" H 10100 2900 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10100 2900 50  0001 C CNN "Link"
	1    10100 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9950 2900 9650 2900
Wire Wire Line
	9650 2900 9650 2700
Connection ~ 9650 2700
Wire Wire Line
	9650 2700 9800 2700
Wire Wire Line
	10250 2900 10500 2900
Wire Wire Line
	10500 2900 10500 2700
Connection ~ 10500 2700
Wire Wire Line
	10500 2700 10900 2700
$Comp
L Jumper:SolderJumper_2_Open SJ104
U 1 1 5FDE3EF6
P 10100 3400
F 0 "SJ104" H 10300 3500 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10100 3514 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10100 3400 50  0001 C CNN
F 3 "" H 10100 3400 50  0001 C CNN
F 4 "np" H 10100 3400 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10100 3400 50  0001 C CNN "Link"
	1    10100 3400
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_2_Open SJ105
U 1 1 5FDE55B9
P 10100 3900
F 0 "SJ105" H 10300 4000 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10100 4014 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10100 3900 50  0001 C CNN
F 3 "" H 10100 3900 50  0001 C CNN
F 4 "np" H 10100 3900 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10100 3900 50  0001 C CNN "Link"
	1    10100 3900
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_2_Open SJ106
U 1 1 5FDE8A8C
P 10100 4400
F 0 "SJ106" H 10300 4500 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10100 4514 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10100 4400 50  0001 C CNN
F 3 "" H 10100 4400 50  0001 C CNN
F 4 "np" H 10100 4400 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10100 4400 50  0001 C CNN "Link"
	1    10100 4400
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_2_Open SJ107
U 1 1 5FDECAA0
P 10100 4950
F 0 "SJ107" H 10300 5050 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10100 5064 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10100 4950 50  0001 C CNN
F 3 "" H 10100 4950 50  0001 C CNN
F 4 "np" H 10100 4950 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10100 4950 50  0001 C CNN "Link"
	1    10100 4950
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_2_Open SJ108
U 1 1 5FDEFEA2
P 10100 5600
F 0 "SJ108" H 10300 5700 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10100 5714 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10100 5600 50  0001 C CNN
F 3 "" H 10100 5600 50  0001 C CNN
F 4 "np" H 10100 5600 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10100 5600 50  0001 C CNN "Link"
	1    10100 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9500 4250 9500 3700
Wire Wire Line
	9500 3700 9650 3700
Wire Wire Line
	9600 4350 9600 4150
Wire Wire Line
	9600 4150 9650 4150
Wire Wire Line
	11450 4300 10700 4300
Wire Wire Line
	10700 4300 10700 3700
Wire Wire Line
	10700 3700 10500 3700
Wire Wire Line
	11450 4400 10600 4400
Wire Wire Line
	10600 4400 10600 4150
Wire Wire Line
	10600 4150 10500 4150
Wire Wire Line
	9950 3400 9650 3400
Wire Wire Line
	9650 3400 9650 3200
Connection ~ 9650 3200
Wire Wire Line
	9650 3200 9800 3200
Wire Wire Line
	10250 3400 10500 3400
Wire Wire Line
	10500 3400 10500 3200
Connection ~ 10500 3200
Wire Wire Line
	10500 3200 10800 3200
Wire Wire Line
	9950 3900 9650 3900
Wire Wire Line
	9650 3900 9650 3700
Connection ~ 9650 3700
Wire Wire Line
	9650 3700 9800 3700
Wire Wire Line
	10250 3900 10500 3900
Wire Wire Line
	10500 3900 10500 3700
Connection ~ 10500 3700
Wire Wire Line
	10500 3700 10400 3700
Wire Wire Line
	9950 4400 9650 4400
Wire Wire Line
	9650 4400 9650 4150
Connection ~ 9650 4150
Wire Wire Line
	9650 4150 9800 4150
Wire Wire Line
	10250 4400 10500 4400
Wire Wire Line
	10500 4400 10500 4150
Connection ~ 10500 4150
Wire Wire Line
	10500 4150 10400 4150
Wire Wire Line
	9950 4950 9650 4950
Wire Wire Line
	9650 4950 9650 4700
Connection ~ 9650 4700
Wire Wire Line
	9650 4700 9800 4700
Wire Wire Line
	10250 4950 10500 4950
Wire Wire Line
	10500 4950 10500 4700
Connection ~ 10500 4700
Wire Wire Line
	10500 4700 10850 4700
Wire Wire Line
	9950 5600 9650 5600
Wire Wire Line
	9650 5600 9650 5250
Connection ~ 9650 5250
Wire Wire Line
	9650 5250 9800 5250
Wire Wire Line
	10250 5600 10500 5600
Wire Wire Line
	10500 5600 10500 5250
Connection ~ 10500 5250
Wire Wire Line
	10500 5250 10950 5250
$Comp
L Jumper:SolderJumper_2_Open SJ101
U 1 1 5FF84D07
P 10350 1250
F 0 "SJ101" H 10350 1363 50  0000 C CNN
F 1 "SolderJumper_2_Open" H 10350 1364 50  0001 C CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 10350 1250 50  0001 C CNN
F 3 "" H 10350 1250 50  0001 C CNN
F 4 "np" H 10350 1250 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 10350 1250 50  0001 C CNN "Link"
	1    10350 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	10200 1250 9950 1250
Wire Wire Line
	9950 1250 9950 1600
Connection ~ 9950 1600
Wire Wire Line
	9950 1600 9750 1600
Wire Wire Line
	10500 1250 10700 1250
Wire Wire Line
	10700 1250 10700 1600
Connection ~ 10700 1600
Wire Wire Line
	10700 1600 10650 1600
$Comp
L Device:R R104
U 1 1 60031369
P 4300 2800
F 0 "R104" V 4400 2800 50  0000 C CNN
F 1 "10k" V 4200 2800 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 4230 2800 50  0001 C CNN
F 3 "" H 4300 2800 50  0001 C CNN
F 4 "311-10.0KCRCT-ND" H 4300 2800 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-10.0KCRCT-ND\">Link</a>" H 4300 2800 60  0001 C CNN "Link"
F 6 "Yageo" V 4300 2800 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-0710KL" V 4300 2800 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 10K OHM 1% 1/8W 0805" V 4300 2800 50  0001 C CNN "Description"
F 9 "C17414" V 4300 2800 50  0001 C CNN "LCSC Part Number"
	1    4300 2800
	0    1    1    0   
$EndComp
$Comp
L Jumper:SolderJumper_2_Open SJ102
U 1 1 60076C09
P 3100 1150
F 0 "SJ102" H 3100 1017 50  0000 C CNN
F 1 "SolderJumper_2_Open" V 3145 1218 50  0001 L CNN
F 2 "Wire_Connections_Bridges:Solder-Jumper-NO-SMD-Pad_Small" H 3100 1150 50  0001 C CNN
F 3 "" H 3100 1150 50  0001 C CNN
F 4 "np" H 3100 1150 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 3100 1150 50  0001 C CNN "Link"
	1    3100 1150
	-1   0    0    1   
$EndComp
Wire Wire Line
	11250 4000 11450 4000
Wire Wire Line
	9300 4050 9000 4050
Wire Wire Line
	9000 4150 9400 4150
Wire Wire Line
	9000 4250 9500 4250
Wire Wire Line
	9000 4350 9600 4350
Wire Wire Line
	9000 4450 9400 4450
Wire Wire Line
	9300 4550 9000 4550
$Comp
L power:VDC #PWR0101
U 1 1 6028A3E9
P 13800 3600
F 0 "#PWR0101" H 13800 3500 50  0001 C CNN
F 1 "VDC" H 13815 3773 50  0000 C CNN
F 2 "" H 13800 3600 50  0001 C CNN
F 3 "" H 13800 3600 50  0001 C CNN
	1    13800 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 950  4300 950 
Wire Wire Line
	3250 1150 3250 2500
$Comp
L Device:R R102
U 1 1 5F9DAE1C
P 2950 1400
F 0 "R102" V 3030 1400 50  0000 C CNN
F 1 "2.2k" V 2850 1400 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 2880 1400 50  0001 C CNN
F 3 "" H 2950 1400 50  0001 C CNN
F 4 "311-2.20KCRCT-ND" H 2950 1400 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-2.20KCRCT-ND\">Link</a>" H 2950 1400 60  0001 C CNN "Link"
F 6 "Yageo" V 2950 1400 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-072K2L" V 2950 1400 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 2.2K OHM 1% 1/8W 0805" V 2950 1400 50  0001 C CNN "Description"
F 9 "C114561" V 2950 1400 50  0001 C CNN "LCSC Part Number"
	1    2950 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2950 1550 2950 1650
Wire Wire Line
	2950 1650 3600 1650
Wire Wire Line
	2950 1250 2950 1150
Wire Wire Line
	2250 2900 2250 4150
Wire Wire Line
	2250 4150 2500 4150
Wire Wire Line
	1650 2900 2250 2900
$Sheet
S -1750 300  1650 1000
U 5FCC6D6A
F0 "Non-PCB-Parts" 39
F1 "Non-PCB-Parts.sch" 39
$EndSheet
Wire Wire Line
	3800 3700 3800 4200
Text Notes 3150 4050 1    39   ~ 0
R105 = 560k for 3.3V Vproc
$Comp
L Connector_Generic:Conn_01x08 J104
U 1 1 5FCE8796
P 11650 4200
F 0 "J104" H 11550 3650 50  0000 L CNN
F 1 "Conn_01x08" V 11730 4101 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x08" H 11650 4200 50  0001 C CNN
F 3 "" H 11650 4200 50  0001 C CNN
F 4 "np" H 11650 4200 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 11650 4200 50  0001 C CNN "Link"
	1    11650 4200
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0149
U 1 1 5FD0C19A
P 11250 3800
F 0 "#PWR0149" H 11250 3550 50  0001 C CNN
F 1 "GNDD" H 11250 3650 50  0000 C CNN
F 2 "" H 11250 3800 50  0000 C CNN
F 3 "" H 11250 3800 50  0000 C CNN
	1    11250 3800
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J105
U 1 1 5FD20ACC
P 12400 4200
F 0 "J105" H 12300 3650 50  0000 L CNN
F 1 "Conn_01x08" V 12480 4101 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x08" H 12400 4200 50  0001 C CNN
F 3 "" H 12400 4200 50  0001 C CNN
F 4 "np" H 12400 4200 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 12400 4200 50  0001 C CNN "Link"
	1    12400 4200
	-1   0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0114
U 1 1 5FEF1181
P 11050 3900
F 0 "#PWR0114" H 11050 3750 50  0001 C CNN
F 1 "+3.3V" H 11065 4073 50  0000 C CNN
F 2 "" H 11050 3900 50  0001 C CNN
F 3 "" H 11050 3900 50  0001 C CNN
	1    11050 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	12600 4100 13400 4100
Wire Wire Line
	13400 4100 13400 2650
Wire Wire Line
	13400 2650 13700 2650
Wire Wire Line
	12600 4200 13450 4200
Wire Wire Line
	13450 4200 13450 2750
Wire Wire Line
	13450 2750 13700 2750
Wire Wire Line
	12600 4300 13500 4300
Wire Wire Line
	13500 4300 13500 2850
Wire Wire Line
	13500 2850 13700 2850
Wire Wire Line
	12600 4400 13550 4400
Wire Wire Line
	13550 4400 13550 2950
Wire Wire Line
	13550 2950 13700 2950
Wire Wire Line
	13700 2550 13400 2550
Wire Wire Line
	14500 2550 14750 2550
$Comp
L power:VDC #PWR0112
U 1 1 600A907A
P 14100 2350
F 0 "#PWR0112" H 14100 2250 50  0001 C CNN
F 1 "VDC" H 14115 2523 50  0000 C CNN
F 2 "" H 14100 2350 50  0001 C CNN
F 3 "" H 14100 2350 50  0001 C CNN
	1    14100 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	14050 3300 14050 3650
Wire Wire Line
	13800 3650 13800 3600
$Comp
L Device:C_Small C110
U 1 1 600B9B46
P 14050 3850
F 0 "C110" V 13950 3700 50  0000 L CNN
F 1 "100nF" V 14150 3700 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 14050 3850 50  0001 C CNN
F 3 "" H 14050 3850 50  0001 C CNN
F 4 "478-1395-1-ND" H 14050 3850 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=478-1395-1-ND\">Link</a>" H 14050 3850 60  0001 C CNN "Link"
F 6 "AVX Corporation" V 14050 3850 50  0001 C CNN "Manufacturer"
F 7 "08055C104KAT2A" V 14050 3850 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 0.1UF 50V X7R 0805" V 14050 3850 50  0001 C CNN "Description"
F 9 "C476766" V 14050 3850 50  0001 C CNN "LCSC Part Number"
	1    14050 3850
	-1   0    0    -1  
$EndComp
Connection ~ 14050 3650
Wire Wire Line
	12600 4500 13600 4500
Wire Wire Line
	13600 4500 13600 3050
Wire Wire Line
	13600 3050 13700 3050
Wire Wire Line
	12600 4600 13650 4600
Wire Wire Line
	13650 4600 13650 3150
Wire Wire Line
	13650 3150 13700 3150
$Comp
L Connector_Generic:Conn_01x01 J109
U 1 1 6011CEB1
P 14250 3650
F 0 "J109" H 14330 3692 50  0000 L CNN
F 1 "Conn_01x01" H 14330 3601 50  0001 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 14250 3650 50  0001 C CNN
F 3 "" H 14250 3650 50  0001 C CNN
F 4 "np" H 14250 3650 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 14250 3650 50  0001 C CNN "Link"
	1    14250 3650
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J110
U 1 1 60130A78
P 13150 4000
F 0 "J110" H 13200 4100 50  0000 L CNN
F 1 "Conn_01x01" H 13230 3951 50  0001 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 13150 4000 50  0001 C CNN
F 3 "" H 13150 4000 50  0001 C CNN
F 4 "np" H 13150 4000 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 13150 4000 50  0001 C CNN "Link"
	1    13150 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	14050 3750 14050 3650
Text Notes 13450 800  0    79   ~ 16
AM MODULATOR BOARD
Text Notes 9050 800  0    79   ~ 16
5V to 3.3V LEVEL SHIFTER FOR AM MODULATOR
Wire Wire Line
	11450 4600 12600 4600
Connection ~ 12600 4600
Wire Wire Line
	11450 4500 12600 4500
Connection ~ 12600 4500
Wire Wire Line
	11450 4400 12600 4400
Connection ~ 12600 4400
Wire Wire Line
	11450 4300 12600 4300
Connection ~ 12600 4300
Wire Wire Line
	11450 4200 12600 4200
Connection ~ 12600 4200
Wire Wire Line
	11450 4100 12600 4100
Connection ~ 12600 4100
Wire Wire Line
	11450 4000 12600 4000
Connection ~ 11450 4000
Wire Wire Line
	11450 3900 12600 3900
Text GLabel 14250 7500 2    39   Output ~ 0
Controlled_Power
Wire Wire Line
	14250 7500 14200 7500
Wire Wire Line
	14200 7500 14200 7700
Wire Wire Line
	14200 7700 14750 7700
Text GLabel 6650 5800 2    39   Output ~ 0
RXD
Text GLabel 7150 5900 2    39   Output ~ 0
TXD
Text GLabel 1650 2600 2    39   Input ~ 0
RXD
Text GLabel 1650 2800 2    39   Input ~ 0
TXD
Text GLabel 14750 7800 0    39   Input ~ 0
EXT_PWR_GROUND
Connection ~ 14200 7700
$Comp
L Connector_Generic:Conn_01x02 J107
U 1 1 5FE800D7
P 14950 8300
F 0 "J107" H 15030 8292 50  0000 L CNN
F 1 "External Power Source" H 15100 8200 50  0000 L CNN
F 2 "Wire_Pads:SolderWirePad_2x_1-5mmDrill_Polarized" H 14950 8300 50  0001 C CNN
F 3 "" H 14950 8300 50  0001 C CNN
F 4 "np" H 14950 8300 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 14950 8300 50  0001 C CNN "Link"
	1    14950 8300
	1    0    0    1   
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0113
U 1 1 5FFE7858
P 9750 1450
F 0 "#PWR0113" H 9750 1300 50  0001 C CNN
F 1 "Vproc" H 9750 1600 50  0000 C CNN
F 2 "" H 9750 1450 50  0000 C CNN
F 3 "" H 9750 1450 50  0000 C CNN
	1    9750 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	13800 3650 13950 3650
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 5FD3E1F6
P 13950 3650
F 0 "#FLG0103" H 13950 3725 50  0001 C CNN
F 1 "PWR_FLAG" H 13950 3823 50  0000 C CNN
F 2 "" H 13950 3650 50  0001 C CNN
F 3 "~" H 13950 3650 50  0001 C CNN
	1    13950 3650
	1    0    0    -1  
$EndComp
Connection ~ 13950 3650
Wire Wire Line
	13950 3650 14050 3650
$Comp
L Diode:ESD9B5.0ST5G D103
U 1 1 5FD2090F
P 700 10150
F 0 "D103" H 700 10071 50  0000 R CNN
F 1 "CG0603MLC-05LECT-ND" V 655 10071 50  0001 R CNN
F 2 "Resistors_SMD:R_0603" H 700 10150 50  0001 C CNN
F 3 "" H 700 10150 50  0001 C CNN
F 4 "CG0603MLC-05LECT-ND" H 700 10150 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CG0603MLC-05LECT-ND\">Link</a>" H 700 10150 50  0001 C CNN "Link"
F 6 "Bourns Inc." H 700 10150 50  0001 C CNN "Manufacturer"
F 7 "CG0603MLC-05LE" H 700 10150 50  0001 C CNN "Manufacturer PN"
F 8 "VARISTOR 0603" H 700 10150 50  0001 C CNN "Description"
F 9 "C316054" H 700 10150 50  0001 C CNN "LCSC Part Number"
	1    700  10150
	0    -1   -1   0   
$EndComp
$Comp
L power:GNDD #PWR0143
U 1 1 5FD45607
P 700 10300
F 0 "#PWR0143" H 700 10050 50  0001 C CNN
F 1 "GNDD" H 700 10150 50  0000 C CNN
F 2 "" H 700 10300 50  0000 C CNN
F 3 "" H 700 10300 50  0000 C CNN
	1    700  10300
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0151
U 1 1 5FD4686E
P 950 10300
F 0 "#PWR0151" H 950 10050 50  0001 C CNN
F 1 "GNDD" H 950 10150 50  0000 C CNN
F 2 "" H 950 10300 50  0000 C CNN
F 3 "" H 950 10300 50  0000 C CNN
	1    950  10300
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0152
U 1 1 5FD477B2
P 1200 10300
F 0 "#PWR0152" H 1200 10050 50  0001 C CNN
F 1 "GNDD" H 1200 10150 50  0000 C CNN
F 2 "" H 1200 10300 50  0000 C CNN
F 3 "" H 1200 10300 50  0000 C CNN
	1    1200 10300
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0153
U 1 1 5FD48905
P 1450 10300
F 0 "#PWR0153" H 1450 10050 50  0001 C CNN
F 1 "GNDD" H 1450 10150 50  0000 C CNN
F 2 "" H 1450 10300 50  0000 C CNN
F 3 "" H 1450 10300 50  0000 C CNN
	1    1450 10300
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0154
U 1 1 5FD4970F
P 1700 10300
F 0 "#PWR0154" H 1700 10050 50  0001 C CNN
F 1 "GNDD" H 1700 10150 50  0000 C CNN
F 2 "" H 1700 10300 50  0000 C CNN
F 3 "" H 1700 10300 50  0000 C CNN
	1    1700 10300
	1    0    0    -1  
$EndComp
Text GLabel 700  10000 1    39   Input ~ 0
RXD
Text GLabel 950  10000 1    39   Input ~ 0
TXD
Text GLabel 1200 10000 1    39   Input ~ 0
TONE_LOGIC
Text GLabel 1450 10000 1    39   Input ~ 0
CW_KEY_LOGIC
Text GLabel 1700 10000 1    39   Input ~ 0
PTT_LOGIC
$Comp
L Device:R R110
U 1 1 5FE40666
P 7000 5900
F 0 "R110" V 7100 5900 50  0000 C CNN
F 1 "10" V 6900 5900 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 6930 5900 50  0001 C CNN
F 3 "" H 7000 5900 50  0001 C CNN
F 4 "311-10.0CRCT-ND" H 7000 5900 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-10.0CRCT-ND\">Link</a>" H 7000 5900 60  0001 C CNN "Link"
F 6 "Yageo" V 7000 5900 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-0710RL" V 7000 5900 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 10 OHM 1% 1/8W 0805" V 7000 5900 50  0001 C CNN "Description"
F 9 "C193989" V 7000 5900 50  0001 C CNN "LCSC Part Number"
	1    7000 5900
	0    -1   1    0   
$EndComp
NoConn ~ 4050 4900
NoConn ~ 4050 5000
$Comp
L Arducon-rescue:Vproc-power #PWR0156
U 1 1 5FDC95A5
P 2750 1950
F 0 "#PWR0156" H 2750 1800 50  0001 C CNN
F 1 "Vproc" V 2750 2200 50  0000 C CNN
F 2 "" H 2750 1950 50  0000 C CNN
F 3 "" H 2750 1950 50  0000 C CNN
	1    2750 1950
	0    1    1    0   
$EndComp
$Comp
L Device:R R108
U 1 1 5FE52722
P 3400 5850
F 0 "R108" V 3300 5850 50  0000 C CNN
F 1 "10k" V 3500 5850 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 3330 5850 50  0001 C CNN
F 3 "" H 3400 5850 50  0001 C CNN
F 4 "311-10.0KCRCT-ND" H 3400 5850 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-10.0KCRCT-ND\">Link</a>" H 3400 5850 60  0001 C CNN "Link"
F 6 "Yageo" V 3400 5850 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-0710KL" V 3400 5850 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 10K OHM 1% 1/8W 0805" V 3400 5850 50  0001 C CNN "Description"
F 9 "C17414" V 3400 5850 50  0001 C CNN "LCSC Part Number"
	1    3400 5850
	-1   0    0    1   
$EndComp
Text GLabel 10050 1700 0    39   Input ~ 0
PWDN
Wire Wire Line
	13800 7950 14450 7950
Wire Wire Line
	14450 7950 14450 8200
Wire Wire Line
	14450 8200 14750 8200
Wire Notes Line
	12100 6700 16200 6700
Wire Notes Line
	16200 6700 16200 8600
Wire Notes Line
	16200 8600 12100 8600
Wire Notes Line
	12100 8600 12100 6700
Wire Notes Line
	8750 850  11950 850 
Wire Notes Line
	11950 850  11950 6000
Wire Notes Line
	11950 6000 8750 6000
Wire Notes Line
	8750 6000 8750 850 
Wire Notes Line
	12150 850  15900 850 
Wire Notes Line
	15900 850  15900 6000
Wire Notes Line
	15900 6000 12150 6000
Wire Notes Line
	12150 6000 12150 850 
$Comp
L Device:C_Small C104
U 1 1 60097870
P 9350 8550
F 0 "C104" V 9450 8450 50  0000 L CNN
F 1 "10uF" V 9250 8450 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 9350 8550 50  0001 C CNN
F 3 "http://www.kemet.com/Lists/ProductCatalog/Attachments/53/KEM_C1002_X7R_SMD.pdf" H 9350 8550 50  0001 C CNN
F 4 "490-5523-1-ND" H 9350 8550 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=490-5523-1-ND\">Link</a>" H 9350 8550 60  0001 C CNN "Link"
F 6 "Murata Electronics" V 9350 8550 50  0001 C CNN "Manufacturer"
F 7 "GRM21BR61E106KA73L" V 9350 8550 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 10UF 25V X5R 0805" V 9350 8550 50  0001 C CNN "Description"
F 9 "C15850" V 9350 8550 50  0001 C CNN "LCSC Part Number"
	1    9350 8550
	0    -1   -1   0   
$EndComp
$Comp
L Arducon-rescue:PN2222A-Transistor_BJT Q101
U 1 1 603D3C3C
P 3950 2800
F 0 "Q101" H 4141 2846 50  0000 L CNN
F 1 "PN2222A" H 4141 2755 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 4150 2725 50  0001 L CIN
F 3 "" H 3950 2800 50  0001 L CNN
F 4 "MMBT2222ATPMSCT-ND" H 3950 2800 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=MMBT2222ATPMSCT-ND\">Link</a>" H 3950 2800 50  0001 C CNN "Link"
F 6 "Micro Commercial Co" H 3950 2800 50  0001 C CNN "Manufacturer"
F 7 "MMBT2222A-TP" H 3950 2800 50  0001 C CNN "Manufacturer PN"
F 8 "TRANS NPN 40V 0.6A SOT23" H 3950 2800 50  0001 C CNN "Description"
F 9 "C181121" H 3950 2800 50  0001 C CNN "LCSC Part Number"
	1    3950 2800
	-1   0    0    -1  
$EndComp
$Comp
L Arducon-rescue:MSS1P4-M3_89A-Diode D109
U 1 1 6037BF00
P 7850 8100
F 0 "D109" H 7850 8200 50  0000 C CNN
F 1 "MSS1P2L-M3/89A" H 8000 8350 50  0001 C CNN
F 2 "Diodes_SMD:DO-219AD" H 7850 7925 50  0001 C CNN
F 3 "" H 7850 8100 50  0001 C CNN
F 4 "MSS1P2L-M3/89AGICT-ND" H 7850 8100 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=MSS1P2L-M3/89AGICT-ND\">Link</a>" H 7850 8100 50  0001 C CNN "Link"
F 6 "Vishay General Semiconductor" H 7850 8100 50  0001 C CNN "Manufacturer"
F 7 "MSS1P2L-M3/89A" H 7850 8100 50  0001 C CNN "Manufacturer PN"
F 8 "DIODE SCHOTTKY 20V 1A MICROSMP" H 7850 8100 50  0001 C CNN "Description"
F 9 "C506621" H 7850 8100 50  0001 C CNN "LCSC Part Number"
	1    7850 8100
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6700 8100 7000 8100
$Comp
L Device:R R111
U 1 1 604113C9
P 9950 9300
F 0 "R111" H 10150 9300 50  0000 C CNN
F 1 "10k" V 9850 9300 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 9880 9300 50  0001 C CNN
F 3 "" H 9950 9300 50  0001 C CNN
F 4 "311-10.0KCRCT-ND" H 9950 9300 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-10.0KCRCT-ND\">Link</a>" H 9950 9300 60  0001 C CNN "Link"
F 6 "Yageo" H 9950 9300 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-0710KL" H 9950 9300 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 10K OHM 1% 1/8W 0805" H 9950 9300 50  0001 C CNN "Description"
F 9 "C17414" H 9950 9300 50  0001 C CNN "LCSC Part Number"
	1    9950 9300
	-1   0    0    1   
$EndComp
Wire Wire Line
	9650 9450 9950 9450
$Comp
L power:PWR_FLAG #FLG0104
U 1 1 60438C92
P 8850 8500
F 0 "#FLG0104" H 8850 8575 50  0001 C CNN
F 1 "PWR_FLAG" H 8850 8673 50  0000 C CNN
F 2 "" H 8850 8500 50  0001 C CNN
F 3 "~" H 8850 8500 50  0001 C CNN
	1    8850 8500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8850 8500 8850 8550
Connection ~ 8850 8550
Wire Wire Line
	8850 8550 9250 8550
Text GLabel 2550 5300 2    39   Input ~ 0
CW_KEY_LOGIC
$Comp
L Device:R R109
U 1 1 603793B1
P 2250 5300
F 0 "R109" V 2350 5300 50  0000 C CNN
F 1 "10k" V 2150 5300 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 2180 5300 50  0001 C CNN
F 3 "" H 2250 5300 50  0001 C CNN
F 4 "311-10.0KCRCT-ND" H 2250 5300 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-10.0KCRCT-ND\">Link</a>" H 2250 5300 60  0001 C CNN "Link"
F 6 "Yageo" V 2250 5300 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-0710KL" V 2250 5300 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 10K OHM 1% 1/8W 0805" V 2250 5300 50  0001 C CNN "Description"
F 9 "C17414" V 2250 5300 50  0001 C CNN "LCSC Part Number"
	1    2250 5300
	0    1    1    0   
$EndComp
$Comp
L Arducon-rescue:PN2222A-Transistor_BJT Q103
U 1 1 6037AAF1
P 1900 5300
F 0 "Q103" H 2091 5346 50  0000 L CNN
F 1 "PN2222A" H 2091 5255 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 2100 5225 50  0001 L CIN
F 3 "" H 1900 5300 50  0001 L CNN
F 4 "MMBT2222ATPMSCT-ND" H 1900 5300 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=MMBT2222ATPMSCT-ND\">Link</a>" H 1900 5300 50  0001 C CNN "Link"
F 6 "Micro Commercial Co" H 1900 5300 50  0001 C CNN "Manufacturer"
F 7 "MMBT2222A-TP" H 1900 5300 50  0001 C CNN "Manufacturer PN"
F 8 "TRANS NPN 40V 0.6A SOT23" H 1900 5300 50  0001 C CNN "Description"
F 9 "C181121" H 1900 5300 50  0001 C CNN "LCSC Part Number"
	1    1900 5300
	-1   0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0142
U 1 1 6037C662
P 1800 5500
F 0 "#PWR0142" H 1800 5250 50  0001 C CNN
F 1 "GNDD" H 1800 5350 50  0000 C CNN
F 2 "" H 1800 5500 50  0000 C CNN
F 3 "" H 1800 5500 50  0000 C CNN
	1    1800 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 5650 3050 5650
Connection ~ 3050 5650
Wire Wire Line
	14550 7950 14450 7950
Connection ~ 14450 7950
Text GLabel 14550 7950 2    39   Output ~ 0
EXTERNAL_PWR_SOURCE
Wire Wire Line
	2600 5650 2500 5650
Wire Wire Line
	3550 5550 4050 5550
Wire Wire Line
	3550 4150 3550 5550
$Comp
L Jumper:SolderJumper_3_Bridged12 SJ110
U 1 1 6050CA09
P 2150 4950
F 0 "SJ110" H 2150 5063 50  0000 C CNN
F 1 "SolderJumper_3_Bridged12" V 2195 5018 50  0001 L CNN
F 2 "Wire_Connections_Bridges:Solder-Jumperx3-NC_1-2_SMD-Pad_Small" H 2150 4950 50  0001 C CNN
F 3 "" H 2150 4950 50  0001 C CNN
F 4 "np" H 2150 4950 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 2150 4950 50  0001 C CNN "Link"
	1    2150 4950
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J106
U 1 1 60513603
P 1300 4500
F 0 "J106" H 1250 4750 50  0000 C CNN
F 1 "Logic Level" V 1400 4450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04" H 1300 4500 50  0001 C CNN
F 3 "" H 1300 4500 50  0001 C CNN
F 4 "np" H 1300 4500 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 1300 4500 50  0001 C CNN "Link"
	1    1300 4500
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1800 5100 1800 4950
Wire Wire Line
	1800 4950 1950 4950
Wire Wire Line
	2150 4800 2150 4600
Wire Wire Line
	2150 4600 1500 4600
Wire Wire Line
	2550 5300 2450 5300
Wire Wire Line
	2350 4950 2450 4950
Wire Wire Line
	2450 4950 2450 5300
Connection ~ 2450 5300
Wire Wire Line
	2450 5300 2400 5300
Wire Wire Line
	6750 4150 6600 4150
Wire Wire Line
	6600 4150 6600 4250
Wire Wire Line
	6250 4500 6600 4500
Wire Wire Line
	6750 4250 6600 4250
Connection ~ 6600 4250
Wire Wire Line
	6600 4250 6600 4500
Wire Wire Line
	6750 4450 6650 4450
Wire Wire Line
	6650 4450 6650 4550
Wire Wire Line
	6250 4600 6650 4600
Wire Wire Line
	6750 4550 6650 4550
Connection ~ 6650 4550
Wire Wire Line
	6650 4550 6650 4600
Wire Wire Line
	6750 4750 6600 4750
Wire Wire Line
	6600 4750 6600 4700
Wire Wire Line
	6250 4700 6600 4700
Wire Wire Line
	6750 4850 6600 4850
Wire Wire Line
	6600 4850 6600 4750
Connection ~ 6600 4750
Wire Wire Line
	6250 5800 6650 5800
Wire Wire Line
	6250 5900 6850 5900
Wire Wire Line
	6250 6200 6400 6200
Wire Wire Line
	6250 6300 6400 6300
NoConn ~ 6400 5300
NoConn ~ 6400 5400
Wire Wire Line
	6250 5600 7000 5600
Wire Wire Line
	6400 6100 6250 6100
$Comp
L Arducon-rescue:PN2222A-Transistor_BJT Q104
U 1 1 6041235C
P 8350 9400
F 0 "Q104" H 8541 9446 50  0000 L CNN
F 1 "PN2222A" H 8541 9355 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 8550 9325 50  0001 L CIN
F 3 "" H 8350 9400 50  0001 L CNN
F 4 "MMBT2222ATPMSCT-ND" H 8350 9400 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=MMBT2222ATPMSCT-ND\">Link</a>" H 8350 9400 50  0001 C CNN "Link"
F 6 "Micro Commercial Co" H 8350 9400 50  0001 C CNN "Manufacturer"
F 7 "MMBT2222A-TP" H 8350 9400 50  0001 C CNN "Manufacturer PN"
F 8 "TRANS NPN 40V 0.6A SOT23" H 8350 9400 50  0001 C CNN "Description"
F 9 "C181121" H 8350 9400 50  0001 C CNN "LCSC Part Number"
	1    8350 9400
	-1   0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR0157
U 1 1 60418738
P 8250 9600
F 0 "#PWR0157" H 8250 9350 50  0001 C CNN
F 1 "GNDD" H 8250 9450 50  0000 C CNN
F 2 "" H 8250 9600 50  0000 C CNN
F 3 "" H 8250 9600 50  0000 C CNN
	1    8250 9600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8750 9550 8750 10150
Wire Wire Line
	8750 10150 10350 10150
Wire Wire Line
	10350 10150 10350 7800
Wire Wire Line
	10350 7800 8600 7800
Wire Wire Line
	8600 7800 8600 8150
Wire Wire Line
	8600 8450 8600 8550
Wire Wire Line
	8600 8550 8850 8550
$Comp
L Device:R R113
U 1 1 604CAF89
P 8700 9400
F 0 "R113" V 8800 9400 50  0000 C CNN
F 1 "210k" V 8600 9400 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" V 8630 9400 50  0001 C CNN
F 3 "" H 8700 9400 50  0001 C CNN
F 4 "311-210KCRCT-ND" H 8700 9400 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-210KCRCT-ND\">Link</a>" H 8700 9400 60  0001 C CNN "Link"
F 6 "Yageo" V 8700 9400 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-07210KL" V 8700 9400 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 210K OHM 1% 1/8W 0805" V 8700 9400 50  0001 C CNN "Description"
F 9 "C170839" V 8700 9400 50  0001 C CNN "LCSC Part Number"
	1    8700 9400
	0    1    1    0   
$EndComp
Wire Wire Line
	8250 9200 8250 9150
NoConn ~ 9650 9150
$Comp
L Switch:SW_SPDT SW102
U 1 1 6042734C
P 2550 2050
F 0 "SW102" H 2750 2050 50  0000 C CNN
F 1 "5V/12V" H 3100 2050 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPDT_PCM12" H 2550 2050 50  0001 C CNN
F 3 "" H 2550 2050 50  0001 C CNN
F 4 "401-2016-1-ND" H 2550 2050 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=401-2016-1-ND\">Link</a>" H 2550 2050 50  0001 C CNN "Link"
F 6 "C&K" H 2550 2050 50  0001 C CNN "Manufacturer"
F 7 "PCM12SMTR" H 2550 2050 50  0001 C CNN "Manufacturer PN"
F 8 "SWITCH SLIDE SPDT 300MA 6V" H 2550 2050 50  0001 C CNN "Description"
F 9 "C221841" H 2550 2050 50  0001 C CNN "LCSC Part Number"
	1    2550 2050
	1    0    0    -1  
$EndComp
Text GLabel 1650 2300 2    39   Output ~ 0
DTR
Text GLabel 4050 5100 0    39   Input ~ 0
DTR
Connection ~ 3250 2500
Text GLabel 1650 2700 2    39   Input ~ 0
CW_KEY
Text GLabel 2250 4600 2    39   Output ~ 0
CW_KEY
Wire Wire Line
	2250 4600 2150 4600
Connection ~ 2150 4600
NoConn ~ 6400 6100
Wire Wire Line
	1650 2100 2000 2100
Wire Wire Line
	2000 2100 2000 1650
Wire Wire Line
	2000 1650 2950 1650
Connection ~ 2950 1650
Wire Wire Line
	2300 2400 2300 2050
Wire Wire Line
	2300 2050 2350 2050
Text GLabel 2350 2400 2    39   Output ~ 0
Power_In
Wire Wire Line
	2350 2400 2300 2400
Text GLabel 2500 5650 0    39   Input ~ 0
Power_In
$Comp
L Connector_Generic:Conn_01x06 J112
U 1 1 604658B7
P 2450 9850
F 0 "J112" H 2368 9325 50  0000 C CNN
F 1 "Conn_01x06" H 2368 9416 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_1x06" H 2450 9850 50  0001 C CNN
F 3 "" H 2450 9850 50  0001 C CNN
F 4 "929550-01-06-ND" H 2450 9850 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=929550-01-06-ND\">Link</a>" H 2450 9850 50  0001 C CNN "Link"
F 6 "TH" H 2450 9850 50  0001 C CNN "Manufacturer"
F 7 "TH" H 2450 9850 50  0001 C CNN "Manufacturer PN"
	1    2450 9850
	-1   0    0    1   
$EndComp
$Comp
L Connector:DB9_Male J111
U 1 1 60486C18
P 3400 9800
F 0 "J111" H 3580 9846 50  0000 L CNN
F 1 "DB9_Male" H 3580 9755 50  0000 L CNN
F 2 "Connect:DB9F_CI" H 3400 9800 50  0001 C CNN
F 3 "" H 3400 9800 50  0001 C CNN
F 4 "609-5908-ND" H 3400 9800 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=609-5908-ND\">Link</a>" H 3400 9800 50  0001 C CNN "Link"
F 6 "TH" H 3400 9800 50  0001 C CNN "Manufacturer"
F 7 "TH" H 3400 9800 50  0001 C CNN "Manufacturer PN"
	1    3400 9800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 10050 2700 10050
Wire Wire Line
	2850 10050 2850 10100
Wire Wire Line
	2850 10100 3100 10100
Wire Wire Line
	2650 9850 2950 9850
Wire Wire Line
	2950 9850 2950 9900
Wire Wire Line
	2950 9900 3100 9900
Wire Wire Line
	2650 9750 2900 9750
Wire Wire Line
	2650 9650 2950 9650
Wire Wire Line
	2650 9550 2800 9550
Wire Wire Line
	2800 9550 2800 10000
Wire Wire Line
	2800 10000 3100 10000
Wire Wire Line
	2650 9950 2700 9950
Wire Wire Line
	2700 9950 2700 10050
Connection ~ 2700 10050
Wire Wire Line
	2700 10050 2850 10050
NoConn ~ 3100 9400
NoConn ~ 3100 9600
NoConn ~ 3100 9800
NoConn ~ 3100 10200
Text Notes 2050 9150 0    79   ~ 16
FTDI PROGRAMMING ADAPTER
Wire Notes Line
	2050 9200 3800 9200
Wire Notes Line
	3800 9200 3800 10400
Wire Notes Line
	3800 10400 2050 10400
Wire Notes Line
	2050 10400 2050 9200
Wire Wire Line
	2900 9750 2900 9700
Wire Wire Line
	2900 9700 3100 9700
Wire Wire Line
	2950 9650 2950 9500
Wire Wire Line
	2950 9500 3100 9500
$Comp
L Device:LED_ALT D102
U 1 1 604C2CC2
P 7250 5350
F 0 "D102" H 7250 5450 50  0000 C CNN
F 1 "LED" H 7250 5250 50  0000 C CNN
F 2 "LEDs:LED_0805" H 7250 5350 50  0001 C CNN
F 3 "" H 7250 5350 50  0001 C CNN
F 4 "732-4984-1-ND" H 7250 5350 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=732-4984-1-ND\">Link</a>" H 7250 5350 50  0001 C CNN "Link"
F 6 "Würth Elektronik" H 7250 5350 50  0001 C CNN "Manufacturer"
F 7 "150080RS75000" H 7250 5350 50  0001 C CNN "Manufacturer PN"
F 8 "LED RED CLEAR 0805 SMD" H 7250 5350 50  0001 C CNN "Description"
F 9 "C389523" H 7250 5350 50  0001 C CNN "LCSC Part Number"
	1    7250 5350
	-1   0    0    1   
$EndComp
$Comp
L power:GNDD #PWR0158
U 1 1 604C760D
P 7400 5350
F 0 "#PWR0158" H 7400 5100 50  0001 C CNN
F 1 "GNDD" H 7400 5200 50  0000 C CNN
F 2 "" H 7400 5350 50  0000 C CNN
F 3 "" H 7400 5350 50  0000 C CNN
	1    7400 5350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6650 5500 6650 5350
Wire Wire Line
	6250 5500 6650 5500
$Comp
L Device:R R114
U 1 1 605220BB
P 6950 5350
F 0 "R114" V 7050 5350 50  0000 C CNN
F 1 "470" V 6850 5350 50  0000 C CNN
F 2 "Resistors_SMD:R_0805" H 6880 5350 50  0001 C CNN
F 3 "" H 6950 5350 50  0001 C CNN
F 4 "311-470CRCT-ND" H 6950 5350 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=311-470CRCT-ND\">Link</a>" H 6950 5350 60  0001 C CNN "Link"
F 6 "Yageo" V 6950 5350 50  0001 C CNN "Manufacturer"
F 7 "RC0805FR-07470RL" V 6950 5350 50  0001 C CNN "Manufacturer PN"
F 8 "RES SMD 470 OHM 1% 1/8W 0805" V 6950 5350 50  0001 C CNN "Description"
F 9 "C328396" V 6950 5350 50  0001 C CNN "LCSC Part Number"
	1    6950 5350
	0    -1   1    0   
$EndComp
Wire Wire Line
	6650 5350 6800 5350
$Comp
L Connector_Generic:Conn_01x04 TP101
U 1 1 606CBB3F
P 7300 4500
F 0 "TP101" V 7218 4680 50  0000 L CNN
F 1 "Conn_01x04" V 7173 4680 50  0001 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04" H 7300 4500 50  0001 C CNN
F 3 "" H 7300 4500 50  0001 C CNN
F 4 "np" H 7300 4500 50  0001 C CNN "Digi-Key Part No."
F 5 "np" H 7300 4500 50  0001 C CNN "Link"
	1    7300 4500
	0    1    -1   0   
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0160
U 1 1 606F1DF4
P 7300 4700
F 0 "#PWR0160" H 7300 4550 50  0001 C CNN
F 1 "Vproc" H 7350 4850 50  0000 C CNN
F 2 "" H 7300 4700 50  0000 C CNN
F 3 "" H 7300 4700 50  0000 C CNN
	1    7300 4700
	1    0    0    1   
$EndComp
$Comp
L Connector:Conn_Coaxial J113
U 1 1 608239D9
P 15150 3100
F 0 "J113" H 15250 3075 50  0000 L CNN
F 1 "To Stub" H 15250 2984 50  0000 L CNN
F 2 "Connect:CON-SMA-EDGE-S" H 15150 3100 50  0001 C CNN
F 3 "" H 15150 3100 50  0001 C CNN
F 4 "CON-SMA-EDGE-S-ND" H 15150 3100 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CON-SMA-EDGE-S-ND\">Link</a>" H 15150 3100 50  0001 C CNN "Link"
F 6 "TH" H 15150 3100 50  0001 C CNN "Manufacturer"
F 7 "TH" H 15150 3100 50  0001 C CNN "Manufacturer PN"
F 8 "TH" H 15150 3100 50  0001 C CNN "Description"
	1    15150 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	14950 3100 14750 3100
Wire Wire Line
	14750 3100 14750 2550
Connection ~ 14750 2550
Wire Wire Line
	14750 2550 14900 2550
$Comp
L Device:Ferrite_Bead_Small FB101
U 1 1 6087D982
P 2000 2200
F 0 "FB101" V 1900 2350 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 1854 2200 50  0001 C CNN
F 2 "Resistors_SMD:R_0603" V 1930 2200 50  0001 C CNN
F 3 "" H 2000 2200 50  0001 C CNN
F 4 "240-2377-1-ND" H 2000 2200 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=240-2377-1-ND\">Link</a>" H 2000 2200 50  0001 C CNN "Link"
F 6 "Laird-Signal Integrity Products" V 2000 2200 50  0001 C CNN "Manufacturer"
F 7 "HZ0603A222R-10" V 2000 2200 50  0001 C CNN "Manufacturer PN"
F 8 "FERRITE BEAD 2.2 KOHM 0603 1LN" V 2000 2200 50  0001 C CNN "Description"
F 9 "C96697" V 2000 2200 50  0001 C CNN "LCSC Part Number"
	1    2000 2200
	0    1    1    0   
$EndComp
Wire Wire Line
	1900 2400 1650 2400
Wire Wire Line
	2100 2400 2300 2400
Connection ~ 2300 2400
Wire Wire Line
	1900 2200 1650 2200
Wire Wire Line
	11050 4000 11050 3900
Wire Wire Line
	11250 3800 11250 3900
Connection ~ 11450 4100
Connection ~ 11450 4200
Connection ~ 11450 4300
Connection ~ 11450 4400
Connection ~ 11450 4500
Connection ~ 11450 4600
Wire Wire Line
	12750 4000 12600 4000
Connection ~ 12600 4000
Wire Wire Line
	12800 3900 12850 3900
Wire Wire Line
	12850 3900 12850 3800
Wire Wire Line
	14500 2950 14600 2950
Connection ~ 14600 2950
Wire Wire Line
	14600 2950 14600 3050
Wire Wire Line
	14400 2650 14600 2650
Wire Wire Line
	14600 2650 14600 2750
Wire Wire Line
	14400 2750 14600 2750
Connection ~ 14600 2750
Wire Wire Line
	14600 2750 14600 2850
Wire Wire Line
	14400 2850 14600 2850
Connection ~ 14600 2850
Wire Wire Line
	14600 2850 14600 2950
Wire Wire Line
	14400 3050 14600 3050
Connection ~ 14600 3050
Wire Wire Line
	14600 3050 14600 3150
Wire Wire Line
	14400 3150 14600 3150
Connection ~ 14600 3150
Wire Wire Line
	14600 3150 14600 3200
$Comp
L power:GND1 #PWR0125
U 1 1 60AE686B
P 12850 3800
F 0 "#PWR0125" H 12850 3550 50  0001 C CNN
F 1 "GND1" H 12855 3627 50  0000 C CNN
F 2 "" H 12850 3800 50  0001 C CNN
F 3 "" H 12850 3800 50  0001 C CNN
	1    12850 3800
	1    0    0    1   
$EndComp
$Comp
L power:GND1 #PWR0111
U 1 1 60B45B22
P 13200 2750
F 0 "#PWR0111" H 13200 2500 50  0001 C CNN
F 1 "GND1" H 13205 2577 50  0000 C CNN
F 2 "" H 13200 2750 50  0001 C CNN
F 3 "" H 13200 2750 50  0001 C CNN
	1    13200 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND1 #PWR0118
U 1 1 60B65B34
P 14600 3200
F 0 "#PWR0118" H 14600 2950 50  0001 C CNN
F 1 "GND1" H 14605 3027 50  0000 C CNN
F 2 "" H 14600 3200 50  0001 C CNN
F 3 "" H 14600 3200 50  0001 C CNN
	1    14600 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND1 #PWR0123
U 1 1 60B86010
P 14250 3350
F 0 "#PWR0123" H 14250 3100 50  0001 C CNN
F 1 "GND1" H 14255 3177 50  0000 C CNN
F 2 "" H 14250 3350 50  0001 C CNN
F 3 "" H 14250 3350 50  0001 C CNN
	1    14250 3350
	1    0    0    -1  
$EndComp
$Comp
L power:GND1 #PWR0117
U 1 1 60BC53FF
P 15100 2750
F 0 "#PWR0117" H 15100 2500 50  0001 C CNN
F 1 "GND1" H 15105 2577 50  0000 C CNN
F 2 "" H 15100 2750 50  0001 C CNN
F 3 "" H 15100 2750 50  0001 C CNN
	1    15100 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND1 #PWR0122
U 1 1 60BC6827
P 15150 3300
F 0 "#PWR0122" H 15150 3050 50  0001 C CNN
F 1 "GND1" H 15155 3127 50  0000 C CNN
F 2 "" H 15150 3300 50  0001 C CNN
F 3 "" H 15150 3300 50  0001 C CNN
	1    15150 3300
	1    0    0    -1  
$EndComp
$Comp
L power:GND1 #PWR0126
U 1 1 60BE7064
P 14050 3950
F 0 "#PWR0126" H 14050 3700 50  0001 C CNN
F 1 "GND1" H 14055 3777 50  0000 C CNN
F 2 "" H 14050 3950 50  0001 C CNN
F 3 "" H 14050 3950 50  0001 C CNN
	1    14050 3950
	1    0    0    -1  
$EndComp
$Comp
L power:GND1 #PWR0119
U 1 1 60C45361
P 13950 3300
F 0 "#PWR0119" H 13950 3050 50  0001 C CNN
F 1 "GND1" H 13955 3127 50  0001 C CNN
F 2 "" H 13950 3300 50  0001 C CNN
F 3 "" H 13950 3300 50  0001 C CNN
	1    13950 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	14250 3300 14250 3350
Wire Wire Line
	14150 3300 14150 3350
Wire Wire Line
	14150 3350 14250 3350
Connection ~ 14250 3350
Wire Wire Line
	2950 3550 2950 3800
$Comp
L Device:Ferrite_Bead_Small FB102
U 1 1 60D39770
P 2000 2400
F 0 "FB102" V 1900 2550 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 1854 2400 50  0001 C CNN
F 2 "Resistors_SMD:R_0603" V 1930 2400 50  0001 C CNN
F 3 "" H 2000 2400 50  0001 C CNN
F 4 "240-2377-1-ND" H 2000 2400 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=240-2377-1-ND\">Link</a>" H 2000 2400 50  0001 C CNN "Link"
F 6 "Laird-Signal Integrity Products" V 2000 2400 50  0001 C CNN "Manufacturer"
F 7 "HZ0603A222R-10" V 2000 2400 50  0001 C CNN "Manufacturer PN"
F 8 "FERRITE BEAD 2.2 KOHM 0603 1LN" V 2000 2400 50  0001 C CNN "Description"
F 9 "C96697" V 2000 2400 50  0001 C CNN "LCSC Part Number"
	1    2000 2400
	0    1    1    0   
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB103
U 1 1 60D5C37A
P 11350 3900
F 0 "FB103" V 11250 4050 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 11204 3900 50  0001 C CNN
F 2 "Resistors_SMD:R_0603" V 11280 3900 50  0001 C CNN
F 3 "" H 11350 3900 50  0001 C CNN
F 4 "240-2377-1-ND" H 11350 3900 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=240-2377-1-ND\">Link</a>" H 11350 3900 50  0001 C CNN "Link"
F 6 "Laird-Signal Integrity Products" V 11350 3900 50  0001 C CNN "Manufacturer"
F 7 "HZ0603A222R-10" V 11350 3900 50  0001 C CNN "Manufacturer PN"
F 8 "FERRITE BEAD 2.2 KOHM 0603 1LN" V 11350 3900 50  0001 C CNN "Description"
F 9 "C96697" V 11350 3900 50  0001 C CNN "LCSC Part Number"
	1    11350 3900
	0    1    1    0   
$EndComp
Connection ~ 11450 3900
$Comp
L Device:Ferrite_Bead_Small FB105
U 1 1 60D818A8
P 11150 4000
F 0 "FB105" V 11050 4150 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 11004 4000 50  0001 C CNN
F 2 "Resistors_SMD:R_0603" V 11080 4000 50  0001 C CNN
F 3 "" H 11150 4000 50  0001 C CNN
F 4 "240-2377-1-ND" H 11150 4000 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=240-2377-1-ND\">Link</a>" H 11150 4000 50  0001 C CNN "Link"
F 6 "Laird-Signal Integrity Products" V 11150 4000 50  0001 C CNN "Manufacturer"
F 7 "HZ0603A222R-10" V 11150 4000 50  0001 C CNN "Manufacturer PN"
F 8 "FERRITE BEAD 2.2 KOHM 0603 1LN" V 11150 4000 50  0001 C CNN "Description"
F 9 "C96697" V 11150 4000 50  0001 C CNN "LCSC Part Number"
	1    11150 4000
	0    1    1    0   
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB104
U 1 1 60DC2016
P 12700 3900
F 0 "FB104" V 12600 3800 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 12554 3900 50  0001 C CNN
F 2 "Resistors_SMD:R_0603" V 12630 3900 50  0001 C CNN
F 3 "" H 12700 3900 50  0001 C CNN
F 4 "240-2377-1-ND" H 12700 3900 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=240-2377-1-ND\">Link</a>" H 12700 3900 50  0001 C CNN "Link"
F 6 "Laird-Signal Integrity Products" V 12700 3900 50  0001 C CNN "Manufacturer"
F 7 "HZ0603A222R-10" V 12700 3900 50  0001 C CNN "Manufacturer PN"
F 8 "FERRITE BEAD 2.2 KOHM 0603 1LN" V 12700 3900 50  0001 C CNN "Description"
F 9 "C96697" V 12700 3900 50  0001 C CNN "LCSC Part Number"
	1    12700 3900
	0    1    1    0   
$EndComp
Connection ~ 12600 3900
$Comp
L Device:Ferrite_Bead_Small FB106
U 1 1 60E037F7
P 12850 4000
F 0 "FB106" V 12750 4150 50  0000 C CNN
F 1 "Ferrite_Bead_Small" V 12704 4000 50  0001 C CNN
F 2 "Resistors_SMD:R_0603" V 12780 4000 50  0001 C CNN
F 3 "" H 12850 4000 50  0001 C CNN
F 4 "240-2377-1-ND" H 12850 4000 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=240-2377-1-ND\">Link</a>" H 12850 4000 50  0001 C CNN "Link"
F 6 "Laird-Signal Integrity Products" V 12850 4000 50  0001 C CNN "Manufacturer"
F 7 "HZ0603A222R-10" V 12850 4000 50  0001 C CNN "Manufacturer PN"
F 8 "FERRITE BEAD 2.2 KOHM 0603 1LN" V 12850 4000 50  0001 C CNN "Description"
F 9 "C96697" V 12850 4000 50  0001 C CNN "LCSC Part Number"
	1    12850 4000
	0    1    1    0   
$EndComp
Wire Wire Line
	3750 6500 4050 6500
$Comp
L Arducon-rescue:MSS1P4-M3_89A-Diode D108
U 1 1 609D6090
P 9250 8300
F 0 "D108" H 9250 8400 50  0000 C CNN
F 1 "MSS1P2L-M3/89A" H 9400 8550 50  0001 C CNN
F 2 "Diodes_SMD:DO-219AD" H 9250 8125 50  0001 C CNN
F 3 "" H 9250 8300 50  0001 C CNN
F 4 "MSS1P2L-M3/89AGICT-ND" H 9250 8300 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=MSS1P2L-M3/89AGICT-ND\">Link</a>" H 9250 8300 50  0001 C CNN "Link"
F 6 "Vishay General Semiconductor" H 9250 8300 50  0001 C CNN "Manufacturer"
F 7 "MSS1P2L-M3/89A" H 9250 8300 50  0001 C CNN "Manufacturer PN"
F 8 "DIODE SCHOTTKY 20V 1A MICROSMP" H 9250 8300 50  0001 C CNN "Description"
F 9 "C506621" H 9250 8300 50  0001 C CNN "LCSC Part Number"
	1    9250 8300
	0    1    -1   0   
$EndComp
$Comp
L Arducon-rescue:MSS1P4-M3_89A-Diode D101
U 1 1 609FC00E
P 3550 6500
F 0 "D101" H 3550 6600 50  0000 C CNN
F 1 "MSS1P2L-M3/89A" H 3700 6750 50  0001 C CNN
F 2 "Diodes_SMD:DO-219AD" H 3550 6325 50  0001 C CNN
F 3 "" H 3550 6500 50  0001 C CNN
F 4 "MSS1P2L-M3/89AGICT-ND" H 3550 6500 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=MSS1P2L-M3/89AGICT-ND\">Link</a>" H 3550 6500 50  0001 C CNN "Link"
F 6 "Vishay General Semiconductor" H 3550 6500 50  0001 C CNN "Manufacturer"
F 7 "MSS1P2L-M3/89A" H 3550 6500 50  0001 C CNN "Manufacturer PN"
F 8 "DIODE SCHOTTKY 20V 1A MICROSMP" H 3550 6500 50  0001 C CNN "Description"
F 9 "C506621" H 3550 6500 50  0001 C CNN "LCSC Part Number"
	1    3550 6500
	-1   0    0    -1  
$EndComp
$Comp
L Diode:ESD9B5.0ST5G D104
U 1 1 60A484E1
P 950 10150
F 0 "D104" H 950 10071 50  0000 R CNN
F 1 "CG0603MLC-05LECT-ND" V 905 10071 50  0001 R CNN
F 2 "Resistors_SMD:R_0603" H 950 10150 50  0001 C CNN
F 3 "" H 950 10150 50  0001 C CNN
F 4 "CG0603MLC-05LECT-ND" H 950 10150 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CG0603MLC-05LECT-ND\">Link</a>" H 950 10150 50  0001 C CNN "Link"
F 6 "Bourns Inc." H 950 10150 50  0001 C CNN "Manufacturer"
F 7 "CG0603MLC-05LE" H 950 10150 50  0001 C CNN "Manufacturer PN"
F 8 "VARISTOR 0603" H 950 10150 50  0001 C CNN "Description"
F 9 "C316054" H 950 10150 50  0001 C CNN "LCSC Part Number"
	1    950  10150
	0    -1   -1   0   
$EndComp
$Comp
L Diode:ESD9B5.0ST5G D105
U 1 1 60A49EA7
P 1200 10150
F 0 "D105" H 1200 10071 50  0000 R CNN
F 1 "CG0603MLC-05LECT-ND" V 1155 10071 50  0001 R CNN
F 2 "Resistors_SMD:R_0603" H 1200 10150 50  0001 C CNN
F 3 "" H 1200 10150 50  0001 C CNN
F 4 "CG0603MLC-05LECT-ND" H 1200 10150 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CG0603MLC-05LECT-ND\">Link</a>" H 1200 10150 50  0001 C CNN "Link"
F 6 "Bourns Inc." H 1200 10150 50  0001 C CNN "Manufacturer"
F 7 "CG0603MLC-05LE" H 1200 10150 50  0001 C CNN "Manufacturer PN"
F 8 "VARISTOR 0603" H 1200 10150 50  0001 C CNN "Description"
F 9 "C316054" H 1200 10150 50  0001 C CNN "LCSC Part Number"
	1    1200 10150
	0    -1   -1   0   
$EndComp
$Comp
L Diode:ESD9B5.0ST5G D106
U 1 1 60A4B3DB
P 1450 10150
F 0 "D106" H 1450 10071 50  0000 R CNN
F 1 "CG0603MLC-05LECT-ND" V 1405 10071 50  0001 R CNN
F 2 "Resistors_SMD:R_0603" H 1450 10150 50  0001 C CNN
F 3 "" H 1450 10150 50  0001 C CNN
F 4 "CG0603MLC-05LECT-ND" H 1450 10150 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CG0603MLC-05LECT-ND\">Link</a>" H 1450 10150 50  0001 C CNN "Link"
F 6 "Bourns Inc." H 1450 10150 50  0001 C CNN "Manufacturer"
F 7 "CG0603MLC-05LE" H 1450 10150 50  0001 C CNN "Manufacturer PN"
F 8 "VARISTOR 0603" H 1450 10150 50  0001 C CNN "Description"
F 9 "C316054" H 1450 10150 50  0001 C CNN "LCSC Part Number"
	1    1450 10150
	0    -1   -1   0   
$EndComp
$Comp
L Diode:ESD9B5.0ST5G D107
U 1 1 60A4CD06
P 1700 10150
F 0 "D107" H 1700 10071 50  0000 R CNN
F 1 "CG0603MLC-05LECT-ND" V 1655 10071 50  0001 R CNN
F 2 "Resistors_SMD:R_0603" H 1700 10150 50  0001 C CNN
F 3 "" H 1700 10150 50  0001 C CNN
F 4 "CG0603MLC-05LECT-ND" H 1700 10150 50  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=CG0603MLC-05LECT-ND\">Link</a>" H 1700 10150 50  0001 C CNN "Link"
F 6 "Bourns Inc." H 1700 10150 50  0001 C CNN "Manufacturer"
F 7 "CG0603MLC-05LE" H 1700 10150 50  0001 C CNN "Manufacturer PN"
F 8 "VARISTOR 0603" H 1700 10150 50  0001 C CNN "Description"
F 9 "C316054" H 1700 10150 50  0001 C CNN "LCSC Part Number"
	1    1700 10150
	0    -1   -1   0   
$EndComp
$Comp
L Timer_RTC:DS3231M U105
U 1 1 60B887F5
P 7000 9550
F 0 "U105" H 7200 10100 61  0000 C CNB
F 1 "DS3231" H 7250 9950 61  0000 C CNB
F 2 "SMD_Packages:SO16" H 7000 8950 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS3231.pdf" H 7270 9600 50  0001 C CNN
F 4 "DS3231SN#T&RCT-ND" H 7000 9550 50  0001 C CNN "Digi-Key Part No."
F 5 "Maxim Integrated" H 7000 9550 50  0001 C CNN "Manufacturer"
F 6 "DS3231SN#T&R" H 7000 9550 50  0001 C CNN "Manufacturer PN"
F 7 "IC RTC CLK/CALENDAR I2C 16-SOIC" H 7000 9550 50  0001 C CNN "Description"
F 8 "<a href=\"https://www.digikey.com/products/en?keywords=DS3231SN#T&RCT-ND\">Link</a>" H 7000 9550 50  0001 C CNN "Link"
F 9 "C9866" H 7000 9550 50  0001 C CNN "LCSC Part Number"
	1    7000 9550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7000 9150 7000 8100
Connection ~ 7000 8100
Wire Wire Line
	7000 8100 7700 8100
$Comp
L power:GNDD #PWR0129
U 1 1 60D1D23C
P 6600 9000
F 0 "#PWR0129" H 6600 8750 50  0001 C CNN
F 1 "GNDD" H 6600 8850 50  0000 C CNN
F 2 "" H 6600 9000 50  0000 C CNN
F 3 "" H 6600 9000 50  0000 C CNN
	1    6600 9000
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C105
U 1 1 60D1D248
P 6700 9000
F 0 "C105" V 6800 8900 50  0000 L CNN
F 1 "10uF" V 6600 8900 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 6700 9000 50  0001 C CNN
F 3 "http://www.kemet.com/Lists/ProductCatalog/Attachments/53/KEM_C1002_X7R_SMD.pdf" H 6700 9000 50  0001 C CNN
F 4 "490-5523-1-ND" H 6700 9000 60  0001 C CNN "Digi-Key Part No."
F 5 "<a href=\"https://www.digikey.com/products/en?keywords=490-5523-1-ND\">Link</a>" H 6700 9000 60  0001 C CNN "Link"
F 6 "Murata Electronics" V 6700 9000 50  0001 C CNN "Manufacturer"
F 7 "GRM21BR61E106KA73L" V 6700 9000 50  0001 C CNN "Manufacturer PN"
F 8 "CAP CER 10UF 25V X5R 0805" V 6700 9000 50  0001 C CNN "Description"
F 9 "C15850" V 6700 9000 50  0001 C CNN "LCSC Part Number"
	1    6700 9000
	0    1    1    0   
$EndComp
$Comp
L Arducon-rescue:Vproc-power #PWR0128
U 1 1 60D66070
P 6900 8900
F 0 "#PWR0128" H 6900 8750 50  0001 C CNN
F 1 "Vproc" H 6900 9050 50  0000 C CNN
F 2 "" H 6900 8900 50  0000 C CNN
F 3 "" H 6900 8900 50  0000 C CNN
	1    6900 8900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 9150 6900 9000
Wire Wire Line
	6800 9000 6900 9000
Connection ~ 6900 9000
Wire Wire Line
	6900 9000 6900 8900
Text GLabel 6500 9350 0    39   Input ~ 0
SCL
Text GLabel 6500 9450 0    39   Input ~ 0
SDA
$Comp
L power:GNDD #PWR0130
U 1 1 60DAA257
P 7000 9950
F 0 "#PWR0130" H 7000 9700 50  0001 C CNN
F 1 "GNDD" H 7000 9800 50  0000 C CNN
F 2 "" H 7000 9950 50  0000 C CNN
F 3 "" H 7000 9950 50  0000 C CNN
	1    7000 9950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 9650 7700 9650
Wire Wire Line
	7700 9650 7700 9150
Wire Wire Line
	7700 9150 7900 9150
Wire Wire Line
	7900 9050 7900 9150
Connection ~ 7900 9150
Wire Wire Line
	7900 9150 8250 9150
NoConn ~ 6500 9750
NoConn ~ 7500 9350
$Comp
L Arducon-rescue:RV-8803-C7-Timer_RTC U103
U 1 1 60FCCE49
P 9250 9300
F 0 "U103" H 9600 9850 60  0000 C CNB
F 1 "RV-8803-C7" H 9800 9750 60  0000 C CNB
F 2 "Oddities:RV-3028-C7" H 9250 9300 60  0001 C CNN
F 3 "https://www.microcrystal.com/fileadmin/Media/Products/RTC/Datasheet/RV-8803-C7.pdf" H 9250 9300 60  0001 C CNN
F 4 "2195-RV-8803-C7-32.768KHZ-3PPM-TA-QCCT-ND" H 9250 9300 50  0001 C CNN "Digi-Key Part No."
F 5 "Micro Crystal AG" H 9250 9300 50  0001 C CNN "Manufacturer"
F 6 "RV-8803-C7-32.768KHZ-3PPM-TA-QC" H 9250 9300 50  0001 C CNN "Manufacturer PN"
F 7 "IC RTC CLK/CALENDAR I2C 8-SON" H 9250 9300 50  0001 C CNN "Description"
F 8 "<a href=\"2195-RV-8803-C7-32.768KHZ-3PPM-TA-QCCT-ND\">Link</a>" H 9250 9300 50  0001 C CNN "Link"
	1    9250 9300
	1    0    0    -1  
$EndComp
Wire Notes Line
	10650 10300 6050 10300
Wire Wire Line
	8000 8100 8300 8100
Wire Wire Line
	8300 8100 8300 8550
Wire Wire Line
	8300 8550 8600 8550
Connection ~ 8600 8550
Wire Notes Line
	7600 8500 6050 8500
Wire Notes Line
	6050 10300 6050 8500
Wire Notes Line
	7600 10300 7600 7650
Wire Notes Line
	7600 7650 10650 7650
Wire Notes Line
	10650 7650 10650 10300
$Comp
L Arducon-rescue:Vproc-power #PWR0121
U 1 1 6099BDD9
P 9950 9150
F 0 "#PWR0121" H 9950 9000 50  0001 C CNN
F 1 "Vproc" H 9950 9300 50  0000 C CNN
F 2 "" H 9950 9150 50  0000 C CNN
F 3 "" H 9950 9150 50  0000 C CNN
	1    9950 9150
	1    0    0    -1  
$EndComp
Wire Bus Line
	6150 4000 6150 6700
$EndSCHEMATC
