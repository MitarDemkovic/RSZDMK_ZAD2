#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

//Velicina prijemnog bafera (mora biti 2^n)
#define USART_RX_BUFFER_SIZE 64

#define BR_KORISNIKA 10

char korisnici[BR_KORISNIKA][32] =

{

    "Sundjer Bob Kockalone",

    "Dijego Armando Maradona",

    "Bond. Dzejms bond.",

    "Zoran Kostic Cane",

    "Kim Dzong Un"

};

char PIN[BR_KORISNIKA][5] = {"5346", "2133", "7445", "8756", "7435"};

char Rx_Buffer[USART_RX_BUFFER_SIZE];			//prijemni FIFO bafer
volatile unsigned char Rx_Buffer_Size = 0;	//broj karaktera u prijemnom baferu
volatile unsigned char Rx_Buffer_First = 0;
volatile unsigned char Rx_Buffer_Last = 0;

ISR(USART_RX_vect)
{
  	Rx_Buffer[Rx_Buffer_Last++] = UDR0;		//ucitavanje primljenog karaktera
	Rx_Buffer_Last &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	if (Rx_Buffer_Size < USART_RX_BUFFER_SIZE)
		Rx_Buffer_Size++;					//inkrement brojaca primljenih karaktera
}

void usartInit(unsigned long baud)
{
	UCSR0A = 0x00;	//inicijalizacija indikatora
					//U2Xn = 0: onemogucena dvostruka brzina
					//MPCMn = 0: onemogucen multiprocesorski rezim

	UCSR0B = 0x98;	//RXCIEn = 1: dozvola prekida izavanog okoncanjem prijema
					//RXENn = 1: dozvola prijema
					//TXENn = 1: dozvola slanja

	UCSR0C = 0x06;	//UMSELn[1:0] = 00: asinroni rezim
					//UPMn[1:0] = 00: bit pariteta se ne koristi
					//USBSn = 0: koristi se jedan stop bit
					//UCSzn[2:0] = 011: 8bitni prenos

	UBRR0 = F_CPU / (16 * baud) - 1;

	sei();	//I = 1 (dozvola prekida)
}

unsigned char usartAvailable()
{
	return Rx_Buffer_Size;		//ocitavanje broja karaktera u prijemnom baferu
}

void usartPutChar(char c)
{
	while(!(UCSR0A & 0x20));	//ceka da se setuje UDREn (indikacija da je predajni bafer prazan)
	UDR0 = c;					//upis karaktera u predajni bafer
}

void usartPutString(char *s)
{
	while(*s != 0)				//petlja se izvrsava do nailaska na nul-terminator
	{
		usartPutChar(*s);		//slanje tekuceg karaktera
		s++;					//azuriranje pokazivaca na tekuci karakter
	}
}

void usartPutString_P(const char *s)
{
	while (1)
	{
		char c = pgm_read_byte(s++);	//citanje sledeceg karaktera iz programske memorije
		if (c == '\0')					//izlazak iz petlje u slucaju
			return;						//nailaska na terminator
		usartPutChar(c);				//slanje karaktera
	}
}

char usartGetChar()
{
	char c;

	if (!Rx_Buffer_Size)						//bafer je prazan?
		return -1;
  	c = Rx_Buffer[Rx_Buffer_First++];			//citanje karaktera iz prijemnog bafera
	Rx_Buffer_First &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	Rx_Buffer_Size--;							//dekrement brojaca karaktera u prijemnom baferu

	return c;
}

unsigned char usartGetString(char *s)
{
	unsigned char len = 0;

	while(Rx_Buffer_Size) 			//ima karaktera u baferu?
		s[len++] = usartGetChar();	//ucitavanje novog karaktera

	s[len] = 0;						//terminacija stringa
	return len;						//vraca broj ocitanih karaktera
}

unsigned char proveraImena(char *s)		//f-ja se veoma cudno ponasa, ako unesemo ime koje je u bazi, sva naredna imena smatra ispravnim, bez obzira da li
										//se nalaze u bazi ili ne
{
	unsigned char test = 0, i = 0, n = 0;
	
	while(n++ <= BR_KORISNIKA)			//petlja u kojoj uzimamo po jednog korisnika iz baze podataka i uporedjujemo ga sa unesenim imenom
	{
		while(korisnici[n][i++])		//petlja u kojoj uzimamo jedno po jedno slovo iz odabranog imena iz baze podataka
		{
			if(s[i] == korisnici[n][i]) //proveravamo slovo po slovo da li se uneseno ime i ime iz baze slazu
				test = 1;				//kada su slova ista test ima vrednost 1
			else
			{
				test = 0;				//ukoliko su slova razlicita test ima vrednost 0 i izlazimo iz druge while petlje kako bismo proverili
				break;					//neko drugo ime iz baze
			}
		}
		
		if(test)						//ako je posle petlje u kojoj proveravamo slovo po slovo test = 1, nema potrebe da proveravamo ostale korisnike
			break;
		
	}
	
	return test;						//vracamo vrednost testa
}

unsigned char proveraPina(char *s)
{
	unsigned char len = 0, test = 0, i = 0, n = 0;

	while(Rx_Buffer_Size) 			//ima karaktera u baferu?
	{
		s[len++] = usartGetChar();	//ucitavanje novog karaktera
		usartPutChar('*');			//ispisivanje * na ekran za svaki uneti karakter
	}
	s[len] = 0;						//terminacija stringa
	
	if(len != 4)
		return 0;					//funkcija vraca 0 ako duzina unetog stringa nije 4 karaktera
	
	
	
}

int main()
{
	usartInit(9600);
	char ime[32], pin[4];
	unsigned char testIme, testPin;
	
	while(1)
	{
		usartPutString("Unesite svoje ime:\r\n");
		while(!usartAvailable());
		_delay_ms(100);
		
		//char ime[32], pin[4];
		//unsigned char testIme, testPin;
		usartGetString(ime);
		testIme = proveraImena(ime);
		/*
		if(testIme == 1)
		{
			unosPina: usartPutString("Unesite vas PIN kod:\r\n");
			while(!usartAvailable());
			_delay_ms(100);
			
			testPin = proveraPina(pin);
			if(testPin == 1)
				usartPutString("Vas pin je ispravan!\r\n");
			else
				goto unosPina;
			
		}
		else
			usartPutString("Vase ime se ne nalazi u bazi podataka!\r\n");
		*/
		if(testIme)
			usartPutString("Ime se nalazi u bazi.\r\n");
		else
			usartPutString("Ime se ne nalazi u bazi.\r\n");
	}

	return 0;
}