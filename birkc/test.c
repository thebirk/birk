typedef struct Entity Entity;
typedef struct Player Player;
typedef struct Enemy Enemy;

typedef enum EntityType EntityType;
enum EntityType {
	EntityPlayer,
	EntityEnemy,
	EntityNameCount,
};

static const char* EntityNames[EntityNameCount] = {
	"EntityPlayer",
	"EntityEnemy",
};

struct Entity {
	EntityType type;
	float x ;
	float y ;
};

struct Player {
	union {
		Entity parent;
		struct {
			EntityType type;
			float x ;
			float y ;
		};
	};
	int health ;
};

struct Enemy {
	union {
		Entity parent;
		struct {
			EntityType type;
			float x ;
			float y ;
		};
	};
	int health ;
	int damage ;
};


Player make_Player() { Player o = {0}; o.type = EntityPlayer; return o;}
Enemy make_Enemy() { Enemy o = {0}; o.type = EntityEnemy; return o;}

#include "stdlib.h"

int main()
{
	Player player = make_Player();
	Entity *e = (Entity*)&player;
	printf("typeof: %s\n", EntityNames[e->type]);

	return 0;
}