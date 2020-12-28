EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 3 3
Title "Raspberry Pi CM4 molliebus carrier"
Date "2020-11-21"
Rev "1"
Comp "WorlickWerx"
Comment1 "CERN-OHL-1.2"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L 00_cm4-compute:ComputeModule4-CM4 Module1
U 1 1 5FC03064
P 5550 1000
F 0 "Module1" H 2700 1347 50  0000 C CNN
F 1 "ComputeModule4-CM4" H 2700 1256 50  0000 C CNN
F 2 "00_cm4-compute:rpiCM4" H 3950 900 50  0001 C CNN
F 3 "" H 3950 900 50  0001 C CNN
F 4 "Hirose" H 2700 1165 50  0000 C CNN "Manufacturer"
F 5 "2x DF40C-100DS-0.4V(51)" H 2700 1074 50  0000 C CNN "Manufacturer Part Number"
	1    5550 1000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5FC29B9C
P 3950 6300
F 0 "#PWR0101" H 3950 6050 50  0001 C CNN
F 1 "GND" H 3955 6127 50  0000 C CNN
F 2 "" H 3950 6300 50  0001 C CNN
F 3 "" H 3950 6300 50  0001 C CNN
	1    3950 6300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 6300 3950 4600
Wire Wire Line
	3950 1100 3900 1100
Wire Wire Line
	3900 1400 3950 1400
Connection ~ 3950 1400
Wire Wire Line
	3950 1400 3950 1100
Wire Wire Line
	3900 1700 3950 1700
Connection ~ 3950 1700
Wire Wire Line
	3950 1700 3950 1400
Wire Wire Line
	3900 2200 3950 2200
Connection ~ 3950 2200
Wire Wire Line
	3950 2200 3950 1700
Wire Wire Line
	3900 2700 3950 2700
Connection ~ 3950 2700
Wire Wire Line
	3950 2700 3950 2200
Wire Wire Line
	3900 3200 3950 3200
Connection ~ 3950 3200
Wire Wire Line
	3950 3200 3950 2700
Wire Wire Line
	3900 3700 3950 3700
Connection ~ 3950 3700
Wire Wire Line
	3950 3700 3950 3200
Wire Wire Line
	3900 4000 3950 4000
Connection ~ 3950 4000
Wire Wire Line
	3950 4000 3950 3700
Wire Wire Line
	3900 4300 3950 4300
Connection ~ 3950 4300
Wire Wire Line
	3950 4300 3950 4000
Wire Wire Line
	3900 4600 3950 4600
Connection ~ 3950 4600
Wire Wire Line
	3950 4600 3950 4300
$Comp
L power:GND #PWR0102
U 1 1 5FC36D46
P 1450 6300
F 0 "#PWR0102" H 1450 6050 50  0001 C CNN
F 1 "GND" H 1455 6127 50  0000 C CNN
F 2 "" H 1450 6300 50  0001 C CNN
F 3 "" H 1450 6300 50  0001 C CNN
	1    1450 6300
	1    0    0    -1  
$EndComp
Wire Wire Line
	1500 1100 1450 1100
Wire Wire Line
	1450 1100 1450 1400
Wire Wire Line
	1500 1400 1450 1400
Connection ~ 1450 1400
Wire Wire Line
	1450 1400 1450 1700
Wire Wire Line
	1500 1700 1450 1700
Connection ~ 1450 1700
Wire Wire Line
	1450 1700 1450 2100
Wire Wire Line
	1500 2100 1450 2100
Connection ~ 1450 2100
Wire Wire Line
	1450 2100 1450 2600
Wire Wire Line
	1500 2600 1450 2600
Connection ~ 1450 2600
Wire Wire Line
	1450 2600 1450 3100
Wire Wire Line
	1500 3100 1450 3100
Connection ~ 1450 3100
Wire Wire Line
	1450 3100 1450 3600
Wire Wire Line
	1500 3600 1450 3600
Connection ~ 1450 3600
Wire Wire Line
	1450 3600 1450 4000
Wire Wire Line
	1500 4000 1450 4000
Connection ~ 1450 4000
Wire Wire Line
	1450 4000 1450 4300
Wire Wire Line
	1500 4300 1450 4300
Connection ~ 1450 4300
Wire Wire Line
	1450 4300 1450 4700
Wire Wire Line
	1500 4700 1450 4700
Connection ~ 1450 4700
Wire Wire Line
	1450 4700 1450 5900
Wire Wire Line
	1500 5900 1450 5900
Connection ~ 1450 5900
Wire Wire Line
	1450 5900 1450 6300
$Comp
L Device:LED D1
U 1 1 5FC40399
P 4550 1350
F 0 "D1" V 4589 1232 50  0000 R CNN
F 1 "LED Green" V 4498 1232 50  0000 R CNN
F 2 "" H 4550 1350 50  0001 C CNN
F 3 "~" H 4550 1350 50  0001 C CNN
	1    4550 1350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3900 2100 4550 2100
Wire Wire Line
	4550 2100 4550 1900
$Comp
L Device:R R1
U 1 1 5FC51414
P 4550 1750
F 0 "R1" H 4620 1796 50  0000 L CNN
F 1 "1K" H 4620 1705 50  0000 L CNN
F 2 "" V 4480 1750 50  0001 C CNN
F 3 "~" H 4550 1750 50  0001 C CNN
	1    4550 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 1500 4550 1600
Wire Wire Line
	4550 1200 4550 950 
Wire Wire Line
	4550 950  4300 950 
Text Label 4300 950  0    50   ~ 0
+3V3
Wire Wire Line
	3900 4900 4400 4900
Text HLabel 4400 4900 2    50   Input ~ 0
+5V
Wire Wire Line
	3900 4900 3900 5000
Connection ~ 3900 4900
Wire Wire Line
	3900 5100 3900 5000
Connection ~ 3900 5000
Wire Wire Line
	3900 5100 3900 5200
Connection ~ 3900 5100
Wire Wire Line
	3900 5200 3900 5300
Connection ~ 3900 5200
Connection ~ 3900 5300
Wire Wire Line
	3900 5300 3900 5400
Wire Wire Line
	1500 5200 1000 5200
Text HLabel 1000 5200 0    50   Output ~ 0
+3V3
Text Label 1100 5200 0    50   ~ 0
+3V3
Wire Wire Line
	1500 5300 1500 5200
Connection ~ 1500 5200
Wire Wire Line
	1500 6000 1000 6000
Text HLabel 1000 6000 0    50   Output ~ 0
~EXTRST~
Wire Wire Line
	3900 6000 4400 6000
Text HLabel 4400 6000 2    50   Input ~ 0
GLOBAL_EN
Wire Wire Line
	1500 5100 1000 5100
Wire Wire Line
	1500 5000 1000 5000
Text HLabel 1000 5000 0    50   Output ~ 0
SCL0
Text HLabel 1000 5100 0    50   Output ~ 0
SDA0
$EndSCHEMATC
