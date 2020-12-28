EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 4 4
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
U 2 1 5FBFD563
P 6650 1450
F 0 "Module1" H 3800 1797 50  0000 C CNN
F 1 "ComputeModule4-CM4" H 3800 1706 50  0000 C CNN
F 2 "00_cm4-compute:rpiCM4" H 5050 1350 50  0001 C CNN
F 3 "" H 5050 1350 50  0001 C CNN
F 4 "Hirose" H 3800 1615 50  0000 C CNN "Manufacturer"
F 5 "2x DF40C-100DS-0.4V(51)" H 3800 1524 50  0000 C CNN "Manufacturer Part Number"
	2    6650 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2250 2100 2250
Wire Wire Line
	2500 2350 2100 2350
Text HLabel 2100 2250 0    50   BiDi ~ 0
PCIE_RX+
Text HLabel 2100 2350 0    50   BiDi ~ 0
PCIE_RX-
Wire Wire Line
	2500 2550 2100 2550
Text HLabel 2100 2550 0    50   BiDi ~ 0
PCIE_TX+
Wire Wire Line
	2500 2650 2100 2650
Text HLabel 2100 2650 0    50   BiDi ~ 0
PCIE_TX-
Wire Wire Line
	2500 2050 2100 2050
Text HLabel 2100 2050 0    50   Output ~ 0
PCIE_CLK-
Wire Wire Line
	2500 1950 2100 1950
Text HLabel 2100 1950 0    50   Output ~ 0
PCIE_CLK+
Wire Wire Line
	2500 1550 2100 1550
Text HLabel 2100 1550 0    50   Input ~ 0
~PCIE_CLK_REQ~
Wire Wire Line
	5100 1950 5500 1950
Text HLabel 5500 1950 2    50   Output ~ 0
~PCIE_RST~
Wire Wire Line
	2500 1850 2450 1850
Wire Wire Line
	2450 6350 2500 6350
Wire Wire Line
	2450 6350 2450 6700
Connection ~ 2450 6350
$Comp
L power:GND #PWR01
U 1 1 5FF10345
P 2450 6700
F 0 "#PWR01" H 2450 6450 50  0001 C CNN
F 1 "GND" H 2455 6527 50  0000 C CNN
F 2 "" H 2450 6700 50  0001 C CNN
F 3 "" H 2450 6700 50  0001 C CNN
	1    2450 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 1850 2450 2150
Wire Wire Line
	2500 6050 2450 6050
Connection ~ 2450 6050
Wire Wire Line
	2450 6050 2450 6350
Wire Wire Line
	2500 5750 2450 5750
Connection ~ 2450 5750
Wire Wire Line
	2450 5750 2450 6050
Wire Wire Line
	2500 5450 2450 5450
Connection ~ 2450 5450
Wire Wire Line
	2450 5450 2450 5750
Wire Wire Line
	2500 5150 2450 5150
Connection ~ 2450 5150
Wire Wire Line
	2450 5150 2450 5450
Wire Wire Line
	2500 4850 2450 4850
Connection ~ 2450 4850
Wire Wire Line
	2450 4850 2450 5150
Wire Wire Line
	2500 4550 2450 4550
Connection ~ 2450 4550
Wire Wire Line
	2450 4550 2450 4850
Wire Wire Line
	2500 4250 2450 4250
Connection ~ 2450 4250
Wire Wire Line
	2450 4250 2450 4550
Wire Wire Line
	2500 3950 2450 3950
Connection ~ 2450 3950
Wire Wire Line
	2450 3950 2450 4250
Wire Wire Line
	2500 3650 2450 3650
Connection ~ 2450 3650
Wire Wire Line
	2450 3650 2450 3950
Wire Wire Line
	2500 3350 2450 3350
Connection ~ 2450 3350
Wire Wire Line
	2450 3350 2450 3650
Wire Wire Line
	2500 3050 2450 3050
Connection ~ 2450 3050
Wire Wire Line
	2450 3050 2450 3350
Wire Wire Line
	2500 2750 2450 2750
Connection ~ 2450 2750
Wire Wire Line
	2450 2750 2450 3050
Wire Wire Line
	2500 2450 2450 2450
Connection ~ 2450 2450
Wire Wire Line
	2450 2450 2450 2750
Wire Wire Line
	2500 2150 2450 2150
Connection ~ 2450 2150
Wire Wire Line
	2450 2150 2450 2450
$Comp
L power:GND #PWR02
U 1 1 5FF171BC
P 5150 6700
F 0 "#PWR02" H 5150 6450 50  0001 C CNN
F 1 "GND" H 5155 6527 50  0000 C CNN
F 2 "" H 5150 6700 50  0001 C CNN
F 3 "" H 5150 6700 50  0001 C CNN
	1    5150 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 6700 5150 6350
Wire Wire Line
	5150 1850 5100 1850
Wire Wire Line
	5100 2150 5150 2150
Connection ~ 5150 2150
Wire Wire Line
	5150 2150 5150 1850
Wire Wire Line
	5100 2450 5150 2450
Connection ~ 5150 2450
Wire Wire Line
	5150 2450 5150 2150
Wire Wire Line
	5100 2750 5150 2750
Connection ~ 5150 2750
Wire Wire Line
	5150 2750 5150 2450
Wire Wire Line
	5100 3050 5150 3050
Connection ~ 5150 3050
Wire Wire Line
	5150 3050 5150 2750
Wire Wire Line
	5100 3350 5150 3350
Connection ~ 5150 3350
Wire Wire Line
	5150 3350 5150 3050
Wire Wire Line
	5100 4250 5150 4250
Connection ~ 5150 4250
Wire Wire Line
	5150 4250 5150 3350
Wire Wire Line
	5100 4550 5150 4550
Connection ~ 5150 4550
Wire Wire Line
	5150 4550 5150 4250
Wire Wire Line
	5100 4850 5150 4850
Connection ~ 5150 4850
Wire Wire Line
	5150 4850 5150 4550
Wire Wire Line
	5100 5150 5150 5150
Connection ~ 5150 5150
Wire Wire Line
	5150 5150 5150 4850
Wire Wire Line
	5100 5450 5150 5450
Connection ~ 5150 5450
Wire Wire Line
	5150 5450 5150 5150
Wire Wire Line
	5100 5750 5150 5750
Connection ~ 5150 5750
Wire Wire Line
	5150 5750 5150 5450
Wire Wire Line
	5100 6050 5150 6050
Connection ~ 5150 6050
Wire Wire Line
	5150 6050 5150 5750
Wire Wire Line
	5100 6350 5150 6350
Connection ~ 5150 6350
Wire Wire Line
	5150 6350 5150 6050
Wire Wire Line
	5100 1550 5500 1550
Text HLabel 5500 1550 2    50   Input ~ 0
USB_OTG_ID
Wire Wire Line
	5100 1650 5500 1650
Text HLabel 5500 1650 2    50   BiDi ~ 0
USB-
Wire Wire Line
	5100 1750 5500 1750
Text HLabel 5500 1750 2    50   BiDi ~ 0
USB+
$EndSCHEMATC
