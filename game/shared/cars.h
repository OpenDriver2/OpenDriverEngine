#ifndef CARS_H
#define CARS_H

typedef short	SHORTVECTOR4[4];
typedef int		LONGVECTOR3[3];
typedef int		LONGVECTOR4[4];
typedef int		LONGQUATERNION[4];

struct CAR_COSMETICS
{
	SVECTOR headLight;
	SVECTOR frontInd, backInd;
	SVECTOR brakeLight, revLight;
	SVECTOR policeLight;
	SVECTOR exhaust, smoke, fire;
	SVECTOR wheelDisp[4];
	short extraInfo;
	short powerRatio;
	short cbYoffset;
	short susCoeff;
	short traction;
	short wheelSize;
	SVECTOR cPoints[12];
	SVECTOR colBox;
	SVECTOR cog;
	short twistRateX, twistRateY, twistRateZ;
	short mass;
};

struct WHEEL
{
	char susCompression;
	char locked;
	char onGrass;
	uint8 surface;
};

struct OrientedBox
{
	VECTOR_NOPAD location;
	SVECTOR_NOPAD radii[3];
	short length[3];
};

typedef struct _HANDLING_DATA
{
	MATRIX where;
	MATRIX drawCarMat;
	LONGVECTOR4 acc;
	LONGVECTOR4 aacc;
	WHEEL wheel[4];
	int wheel_speed, speed;
	int direction;
	int front_vel, rear_vel;
	int mayBeColliding;		// [A] now used as a bitfield to create collision pairs
	short revs;
	char gear, changingGear;
	char autoBrake;

	OrientedBox oBox;
} HANDLING_DATA;

union RigidBodyState
{
	int v[13];
	struct {
		LONGVECTOR3 fposition;
		LONGQUATERNION orientation;
		LONGVECTOR3 linearVelocity;
		LONGVECTOR3 angularVelocity;
	} n;
};

typedef struct _APPEARANCE_DATA
{
	SXYPAIR light_trails[4][4];
	CAR_COSMETICS* carCos;
	short old_clock[4];
	char life;
	char coplife;
	short qy, qw;
	char life2;
	char model;
	char palette;

	char needsDenting : 1;
	char flags : 7;			// [A] new: appearance flags, 1,2,3,4 = wheel hubcaps lost

	short damage[6];
} APPEARANCE_DATA;

struct CIV_ROUTE_ENTRY
{
	short dir;
	uint16 pathType;
	int distAlongSegment;
	int x;
	int z;
};

struct CIV_STATE
{
	int currentRoad;
	int currentNode;
	CIV_ROUTE_ENTRY* ctrlNode;
	uint8 ctrlState;
	uint8 trafficLightPhaseId;
	uint8 changeLane;
	uint8 turnDir;
	char brakeLight;
	uint8 oldLane;
	uint8 changeLaneCount;
	uint8 pad3;
	int turnNode;
	int changeLaneIndicateCount;
	int carPauseCnt;
	int velRatio;
	CIV_ROUTE_ENTRY targetRoute[13];
	CIV_ROUTE_ENTRY* pnode;
	uint8 maxSpeed;
	uint8 thrustState;
	uint8 carMustDie;
	uint8 currentLane;
};

struct COP
{
	XZPAIR targetHistory[2];
	char routeInMemory;
	char justPinged;
	char close_pursuit;
	char dying;
	uint16 DistanceToPlayer;
	short desiredSpeed;
	short recalcTimer;
	short stuckTimer;
	short lastRecoverStrategy;
	short recoveryTimer;
	short hiddenTimer;
	short frontLClear, frontRClear;
	short batterTimer;	// [A] new gameplay feature
};

struct LEAD_CAR
{
	char dstate;
	char ctt;
	short targetDir;
	int targetX;
	int targetZ;
	int currentRoad;
	int lastRoad;
	int nextJunction;
	int nextTurn;
	int nextExit;
	int stuckCount;
	int panicCount;
	int recoverTime;
	int roadPosition;
	int roadForward;
	int boringness;
	int avoid;
	int lastTarget;
	int offRoad;
	int width;
	int d;
	int base_Normal;
	int base_Angle;
	int base_Dir;
	int outsideSpoolRegion;
	int direction;
	int lastDirection;
	char takeDamage;
};

enum ECarControlType
{
	CONTROL_TYPE_NONE = 0,				// car is not in the world
	CONTROL_TYPE_PLAYER = 1,			// controlled by player pads
	CONTROL_TYPE_CIV_AI = 2,			// Civilian car. May be a passive cop car with CONTROL_FLAG_COP flag.
	CONTROL_TYPE_PURSUER_AI = 3,		// Police pursuer car. Always chases player
	CONTROL_TYPE_LEAD_AI = 4,			// FreeRoamer AI

	CONTROL_TYPE_CAMERACOLLIDER = 5,	// Used as a camera collider
	CONTROL_TYPE_TANNERCOLLIDER = 6,	// Used as collision box for tanner
	CONTROL_TYPE_CUTSCENE = 7,			// Pretty same as player car but controllled by cutscene. Can be a chase car.
};

enum ECarControlFlags
{
	CONTROL_FLAG_COP = (1 << 0),				// civ car is a cop car
	CONTROL_FLAG_COP_SLEEPING = (1 << 1),		// passive cop flag (roadblocks). Hitting car with that flag results it's activation
	CONTROL_FLAG_WAS_PARKED = (1 << 2),			// car pinged in as parked. Really nothing to do with it
	CONTROL_FLAG_PLAYER_START_CAR = (1 << 3),	// car owned by player
};
//------------------------------------------------------------------

class CCar
{
	friend class CManager_Cars;
public:
					CCar();
					~CCar();

	// handling
	void			InitCarPhysics(LONGVECTOR4* startpos, int direction);
	void			TempBuildHandlingMatrix(int init);
	void			StepCarPhysics();

	void			CheckCarEffects();

	// drawing
	void			UpdateCarDrawMatrix();
	void			DentCar();
	void			DrawCar();

	// wheel forces
	void			StepOneCar();

protected:

	struct CAR_LOCALS
	{
		LONGVECTOR4 vel;
		LONGVECTOR4 avel;
		int extraangulardamping;
		int aggressive;
	};

	void			CreateDentableCar();

	// handling
	void			InitOrientedBox();
	void			RebuildCarMatrix(RigidBodyState& st);

	void			JumpDebris();
	void			NoseDown();

	// game sound
	uint16			GetEngineRevs();
	void			ControlCarRevs();

	// wheel forces
	void			GetFrictionScalesDriver1(CAR_LOCALS& cl, int& frontFS, int& rearFS);
	void			AddWheelForcesDriver1(CAR_LOCALS& cl);
	void			ConvertTorqueToAngularAcceleration(CAR_LOCALS& cl);

	// --------------------
	HANDLING_DATA	m_hd;
	RigidBodyState	m_st;
	APPEARANCE_DATA	m_ap;

	uint8			m_hndType{ 0 };

	uint8			m_controlType{ 0 };
	uint8			m_controlFlags{ 0 };

	int8			m_id{ -1 };

	union {
		char* padid{ 0 };		// CONTROL_TYPE_PLAYER or CONTROL_TYPE_CUTSCENE
		CIV_STATE c;		// CONTROL_TYPE_CIV_AI
		COP p;				// CONTROL_TYPE_PURSUER_AI
		LEAD_CAR l;			// CONTROL_TYPE_LEAD_AI
	} m_ai;

	int*			m_inform{ nullptr };

	short			m_thrust{ 0 };
	short			m_felonyRating{ 0 };

	int8			m_handbrake{ 0 };
	int8			m_wheelspin{ 0 };
	int8			m_wasOnGround{ 0 };
	int8			m_lowDetail{ 0 };

	short			m_wheel_angle{ 0 };
	uint16			m_totalDamage{ 0 };

	int				m_lastPad{ 0 };

	int				m_frontWheelRotation{ 0 };
	int				m_backWheelRotation{ 0 };
};

#endif // CARS_H