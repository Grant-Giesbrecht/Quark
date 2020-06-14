// This is a comment

ADD
SUB

WHILECARRY{ //This is an inline comment
	SUB
	ADD
	//Comment
}

FAN0 1

IFCARRY{
	RPLAN 1 //Reprogrammable light on
}ELSE{
	RPAUS 1 //Reprogrammable light off
}

FAN0 0

IFZERO{
	RAM_DISPA 0x14
}
