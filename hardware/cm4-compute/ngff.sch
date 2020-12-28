EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 3 4
Title "M.2 slot M expansion"
Date "2020-12-28"
Rev "1"
Comp "WorlickWerx"
Comment1 "CERN-OHL-1.2"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	8400 3550 8200 3550
Wire Wire Line
	8400 3450 8200 3450
Wire Wire Line
	8400 3650 8200 3650
Wire Wire Line
	8400 3750 8200 3750
Wire Wire Line
	8400 4350 8200 4350
Wire Wire Line
	8400 3950 8200 3950
Wire Wire Line
	8400 4050 8200 4050
Wire Wire Line
	8400 4250 8200 4250
Text HLabel 8200 3550 0    50   BiDi ~ 0
PCIE_RX+
Text HLabel 8200 3450 0    50   BiDi ~ 0
PCIE_RX-
Text HLabel 8200 3650 0    50   BiDi ~ 0
PCIE_TX-
Text HLabel 8200 3750 0    50   BiDi ~ 0
PCIE_TX+
Text HLabel 8200 4050 0    50   Input ~ 0
PCIE_CLK+
Text HLabel 8200 3950 0    50   Input ~ 0
PCIE_CLK-
Text HLabel 8200 4250 0    50   Input ~ 0
~PCIE_RST~
Text HLabel 8200 4350 0    50   Output ~ 0
~PCIE_CLK_REQ~
Text Notes 7650 6000 0    50   ~ 0
Notes:\n- Common form factor is 2280 (22mm W x 80mm L)\n- Key M needed for NVME SSD (key at pin 59-66)\n- Max 2.5A @ 3V3 per PCI-SIG M.2 spec v1\n- Samsung EVO-Plus 2TB rated at 6W max (3V3 @ 1.81A)\n- PCIe Tx and Rx may be reversed when connected to host
Wire Wire Line
	9350 3750 9400 3750
Wire Wire Line
	9400 3750 9400 3850
$Comp
L power:GND #PWR03
U 1 1 5FFA237B
P 9400 5050
F 0 "#PWR03" H 9400 4800 50  0001 C CNN
F 1 "GND" H 9405 4877 50  0000 C CNN
F 2 "" H 9400 5050 50  0001 C CNN
F 3 "" H 9400 5050 50  0001 C CNN
	1    9400 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	9350 3850 9400 3850
Connection ~ 9400 3850
Wire Wire Line
	9400 3850 9400 3950
Wire Wire Line
	9350 3950 9400 3950
Connection ~ 9400 3950
Wire Wire Line
	9400 3950 9400 4050
Wire Wire Line
	9350 4050 9400 4050
Connection ~ 9400 4050
Wire Wire Line
	9400 4050 9400 4150
Wire Wire Line
	9350 4150 9400 4150
Connection ~ 9400 4150
Wire Wire Line
	9400 4150 9400 4250
Wire Wire Line
	9350 4250 9400 4250
Connection ~ 9400 4250
Wire Wire Line
	9400 4250 9400 4350
Wire Wire Line
	9350 4350 9400 4350
Connection ~ 9400 4350
Wire Wire Line
	9400 4350 9400 4450
Wire Wire Line
	9350 4450 9400 4450
Connection ~ 9400 4450
Wire Wire Line
	9400 4450 9400 4550
Wire Wire Line
	9350 4550 9400 4550
Connection ~ 9400 4550
Wire Wire Line
	9400 4550 9400 4650
Wire Wire Line
	9350 4650 9400 4650
Connection ~ 9400 4650
Wire Wire Line
	9400 4650 9400 4750
Wire Wire Line
	9350 4750 9400 4750
Connection ~ 9400 4750
Wire Wire Line
	9400 4750 9400 4850
$Comp
L Device:R R1
U 1 1 5FFA4D51
P 8150 4650
F 0 "R1" V 8100 4450 50  0000 C CNN
F 1 "10K" V 8150 4650 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 8080 4650 50  0001 C CNN
F 3 "~" H 8150 4650 50  0001 C CNN
	1    8150 4650
	0    1    1    0   
$EndComp
Wire Wire Line
	8300 4650 8400 4650
Wire Wire Line
	8000 4650 7450 4650
Text Label 7450 4650 0    50   ~ 0
PCIE_3V3
Wire Wire Line
	9350 3550 9400 3550
Wire Wire Line
	9400 3550 9400 3650
Connection ~ 9400 3750
Wire Wire Line
	9350 3650 9400 3650
Connection ~ 9400 3650
Wire Wire Line
	9400 3650 9400 3750
Wire Wire Line
	9350 2750 9400 2750
Wire Wire Line
	9400 2750 9400 2650
Wire Wire Line
	9350 1950 9400 1950
Connection ~ 9400 1950
Wire Wire Line
	9400 1950 9400 1700
Wire Wire Line
	9350 2050 9400 2050
Connection ~ 9400 2050
Wire Wire Line
	9400 2050 9400 1950
Wire Wire Line
	9350 2150 9400 2150
Connection ~ 9400 2150
Wire Wire Line
	9400 2150 9400 2050
Wire Wire Line
	9350 2250 9400 2250
Connection ~ 9400 2250
Wire Wire Line
	9400 2250 9400 2150
Wire Wire Line
	9350 2350 9400 2350
Connection ~ 9400 2350
Wire Wire Line
	9400 2350 9400 2250
Wire Wire Line
	9350 2450 9400 2450
Connection ~ 9400 2450
Wire Wire Line
	9400 2450 9400 2350
Wire Wire Line
	9350 2550 9400 2550
Connection ~ 9400 2550
Wire Wire Line
	9400 2550 9400 2450
Wire Wire Line
	9350 2650 9400 2650
Connection ~ 9400 2650
Wire Wire Line
	9400 2650 9400 2550
Wire Wire Line
	9400 1700 9800 1700
Text Label 9800 1700 2    50   ~ 0
PCIE_3V3
NoConn ~ 8400 1950
NoConn ~ 8400 2050
NoConn ~ 8400 2150
NoConn ~ 8400 2250
NoConn ~ 8400 2450
NoConn ~ 8400 2550
NoConn ~ 8400 2650
NoConn ~ 8400 2750
NoConn ~ 8400 2950
NoConn ~ 8400 3050
NoConn ~ 8400 3150
NoConn ~ 8400 3250
NoConn ~ 8400 4850
NoConn ~ 8400 4550
$Comp
L 00_cm4-compute:M.2_Key_M_Socket_3_PCIe J1
U 1 1 5FFD81A2
P 8850 3050
F 0 "J1" H 8875 4415 50  0000 C CNN
F 1 "M.2_Key_M_Socket_3_PCIe" H 8875 4324 50  0000 C CNN
F 2 "00_cm4-compute:Conn_TE-M.2-0.5-67P-doublesided" H 9500 4400 50  0001 C CNN
F 3 "" H 8450 2700 50  0001 C CNN
	1    8850 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	9350 4850 9400 4850
Connection ~ 9400 4850
Wire Wire Line
	9400 4850 9400 5050
NoConn ~ 9350 3250
NoConn ~ 9350 3350
NoConn ~ 8400 4750
$Comp
L Mechanical:MountingHole_Pad H3
U 1 1 5FFF106A
P 10250 3600
F 0 "H3" H 10350 3603 50  0000 L CNN
F 1 "MountingHole_Pad" H 10350 3558 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2_DIN965_Pad_TopBottom" H 10250 3600 50  0001 C CNN
F 3 "~" H 10250 3600 50  0001 C CNN
	1    10250 3600
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H2
U 1 1 5FFF1D58
P 9950 3600
F 0 "H2" H 10050 3603 50  0000 L CNN
F 1 "MountingHole_Pad" H 10050 3558 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2_DIN965_Pad_TopBottom" H 9950 3600 50  0001 C CNN
F 3 "~" H 9950 3600 50  0001 C CNN
	1    9950 3600
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H1
U 1 1 5FFF244B
P 9650 3600
F 0 "H1" H 9750 3603 50  0000 L CNN
F 1 "MountingHole_Pad" H 9750 3558 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2_DIN965_Pad_TopBottom" H 9650 3600 50  0001 C CNN
F 3 "~" H 9650 3600 50  0001 C CNN
	1    9650 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5FFF2F7D
P 9650 3750
F 0 "#PWR0103" H 9650 3500 50  0001 C CNN
F 1 "GND" H 9655 3577 50  0000 C CNN
F 2 "" H 9650 3750 50  0001 C CNN
F 3 "" H 9650 3750 50  0001 C CNN
	1    9650 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	10250 3700 9950 3700
Connection ~ 9950 3700
Wire Wire Line
	9950 3700 9650 3700
Wire Wire Line
	9650 3750 9650 3700
Connection ~ 9650 3700
$EndSCHEMATC
