typedef struct{
   char*    caption;
   boolean    state;
   unsigned long   unix;
 } RELAY;

 typedef struct {
  char* caption;
  boolean state;
  float fValue;
  float fPValue;
  int iValue;
  int iPValue;
  unsigned long lastRead;
 } SENSOR;

 typedef struct {
  char *caption;
  int HourStart;
  int HourStop;
  int Led12Percent;
  int Led24Every;
  int Led24During;
  int PumpEvery;
  int PumpDuring;
  float TempChange;
  float MoistChange;
 }CONFIGMODE;

