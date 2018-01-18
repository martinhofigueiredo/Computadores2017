#include "Arduino.h"
#include "Wire.h"
#define endereco_RTC  0x68
#define endereco_Temperatura_MSB  0x11
#define endereco_Temperatura_LSB  0x12

#define BUFSIZE 10
char cmd[BUFSIZE];
int pos = 0;

byte temp_msb, temp_lsb;
byte segundos = 0, minutos = 0, horas = 0, diaSemana = 0, diaMes = 0, mes = 0, ano = 0;

// Converte números em decimal para binário
byte decBin(byte val) {return( (val/10*16) + (val%10) );}
// Converte números em binário para decimal
byte binDec(byte val) {return( (val/16*10) + (val%16) );}

void setup()
{
  Wire.begin();
  Serial.begin(9600);
}

byte leByte()
{
  while (!Serial.available());
  byte reading = 0;
  byte incomingByte = Serial.read();
  while (incomingByte != '\n')
  {
    if (incomingByte >= '0' && incomingByte <= '9')
      reading = reading * 10 + (incomingByte - '0');
    else;
    incomingByte = Serial.read();
  }
  Serial.flush();
  Serial.print("\n");
  return reading;
}

void write_horas(byte segundos, byte minutos, byte horas, byte diaSemana, byte diaMes, byte mes, byte ano)
{
  // acerta hora e data
  Wire.beginTransmission(endereco_RTC);
  Wire.write(0); // configura de forma ao 1º byte acertar os segundos
  Wire.write(decBin(segundos)); // acerta segundos
  Wire.write(decBin(minutos)); // acerta minutos
  Wire.write(decBin(horas)); // acerta horas
  Wire.write(decBin(diaSemana)); // acerta diaSemana (1=Domingo, 7=Sabado)
  Wire.write(decBin(diaMes)); // acerta diaMes (1 a 31)
  Wire.write(decBin(mes)); // acerta mes
  Wire.write(decBin(ano)); // acerta ano (0 a 99)
  Wire.endTransmission();
}

void read_horas(byte *segundos, byte *minutos, byte *horas, byte *diaSemana, byte *diaMes, byte *mes, byte *ano)
{
  Wire.beginTransmission(endereco_RTC);
  Wire.write(0); // muda registo do RTC para 00h
  Wire.endTransmission();
  Wire.requestFrom(endereco_RTC, 7);
  // 7 bytes de dados a começar no registo 00h
  *segundos = binDec(Wire.read() & 0x7f);
  *minutos = binDec(Wire.read());
  *horas = binDec(Wire.read() & 0x3f);
  *diaSemana = binDec(Wire.read());
  *diaMes = binDec(Wire.read());
  *mes = binDec(Wire.read());
  *ano = binDec(Wire.read());
}
void print_horas()
{

  read_horas(&segundos, &minutos, &horas, &diaSemana, &diaMes, &mes,
  &ano);

  Serial.print(horas, DEC);

  Serial.print(":");
  if (minutos<10)
  {
    Serial.print("0");
  }
  Serial.print(minutos, DEC);
  Serial.print(":");
  if (segundos<10)
  {
    Serial.print("0");
  }
  Serial.print(segundos, DEC);
  Serial.print(" ");
  if (diaMes<10)
  {
    Serial.print("0");
  }
  Serial.print(diaMes, DEC);
  Serial.print("/");
  if (mes<10)
  {
    Serial.print("0");
  }
  Serial.print(mes, DEC);
  Serial.print("/");
  if (ano<10)
  {
    Serial.print("0");
  }
  Serial.print(ano, DEC);
  Serial.print(" Dia da Semana: ");
  switch(diaSemana)
  {
    case 1:
      Serial.println("Domingo");
      break;
    case 2:
      Serial.println("Segunda");
      break;
    case 3:
      Serial.println("Terça");
      break;
    case 4:
      Serial.println("Quarta");
      break;
    case 5:
      Serial.println("Quinta");
      break;
    case 6:
      Serial.println("Sexta");
      break;
    case 7:
      Serial.println("Sabado");
      break;
 }
}

byte le_Temp_MSB()
{
  Wire.beginTransmission(endereco_RTC);
  Wire.write(endereco_Temperatura_MSB);
  Wire.endTransmission();

  Wire.requestFrom(endereco_RTC, 1);
  temp_msb = Wire.read();

}

byte le_Temp_LSB(){

  Wire.beginTransmission(endereco_RTC);
  Wire.write(endereco_Temperatura_LSB);
  Wire.endTransmission();

  Wire.requestFrom(endereco_RTC, 1);
  temp_lsb = Wire.read() >> 6;


}

void loop()
{
  if (Serial.available())
  {
    char ch = Serial.read();
    Serial.write(ch);
    if (ch == '\n')
    {
      switch (cmd[0])
      {
        case 'a': // Acerta Horas

        Serial.println("Horas (0 a 24): ");
        horas = leByte();

        Serial.println("Minutos (0 a 60): ");
        minutos = leByte();

        Serial.println("Segundos (0 a 60): ");
        segundos = leByte();

        Serial.println("Dia da Semana (1 = Domingo a 7 = Sabado): ");
        diaSemana = leByte();

        Serial.println("Dia (0 a 31):");
        diaMes = leByte();

        Serial.println("Mes (0 a 12):");
        mes = leByte();

        Serial.println("Ano (0 a 99):");
        ano = leByte();

        write_horas(segundos, minutos, horas, diaSemana, diaMes, mes, ano);

        Serial.println("Data e Hora alterada para: ");
        print_horas();
        break;

        case 'i': // Imprime Hora/Data
        print_horas();
        break;

        case 't': // imprime Temperatura
        temp_msb = le_Temp_MSB();
        temp_lsb = le_Temp_LSB();

        Serial.print(temp_msb);
        Serial.print(".");
        Serial.print(binDec(temp_lsb));
        Serial.println(" ºC");
      }
      pos = 0;
    } else
      if ((pos < BUFSIZE) && (ch >= ' '))
        cmd[pos++] = ch;
  }
}
