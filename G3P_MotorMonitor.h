
typedef struct {
  bool en;
  int hlfb;
} MotorStatus;

typedef struct {
  MotorStatus x;
  MotorStatus xp;
  MotorStatus y;
  MotorStatus z;
  MotorStatus a;
} Motors;

