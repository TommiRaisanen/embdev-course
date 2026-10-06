*** Settings ***
Library   String
Library   SerialLibrary

*** Variables ***
${com}   	COM5
${baud} 	115200
${board}	nRF5340

*** Test Cases ***
Connect Serial
	Log To Console  Connecting to ${board}
	Add Port  ${com}  baudrate=${baud}  encoding=ascii
	Port Should Be Open  ${com}
	Reset Input Buffer
	Reset Output Buffer

Valid Time String
	${sent} =      Set Variable    000120X
	${expected} =  Set Variable    80X
	

	Write Data   ${sent}   encoding=ascii
	Log To Console   Send ${sent}

	${read} =   Read Until   terminator=58   encoding=ascii
	Log To Console   Received ${read}

	Should Be Equal As Strings   ${read}    ${expected}
	Log To Console   Tested ${read} is same as ${expected}

Invalid Time String
	${sent} =      Set Variable    001067X
	${expected} =  Set Variable    -3X

	Write Data   ${sent}   encoding=ascii
	Log To Console   Send ${sent}

	${read} =   Read Until   terminator=58   encoding=ascii
	Log To Console   Received ${read}

	Should Be Equal As Strings   ${read}    ${expected}
	Log To Console   Tested ${read} is same as ${expected}

Length Check
	Reset Input Buffer
	${sent} =      Set Variable    0001X
	${expected} =  Set Variable    -1X
	Write Data   ${sent}   encoding=ascii
	Log To Console   Send ${sent}
	${read} =   Read Until   terminator=58   encoding=ascii
	Log To Console   Received ${read}
	Should Be Equal As Strings   ${read}    ${expected}

Zero Seconds Check
	Reset Input Buffer
	${sent} =      Set Variable    000000X
	${expected} =  Set Variable    -6X
	Write Data   ${sent}   encoding=ascii
	Log To Console   Send ${sent}
	${read} =   Read Until   terminator=58   encoding=ascii
	Log To Console   Received ${read}
	Should Be Equal As Strings   ${read}    ${expected}

Digit Check
	Reset Input Buffer
	${sent} =      Set Variable    00A120X
	${expected} =  Set Variable    -3X
	Write Data   ${sent}   encoding=ascii
	Log To Console   Send ${sent}
	${read} =   Read Until   terminator=58   encoding=ascii
	Log To Console   Received ${read}
	Should Be Equal As Strings   ${read}    ${expected}

Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}