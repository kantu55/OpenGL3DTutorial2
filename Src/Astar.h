#ifndef ASTAR_H_INCLUDED
#define ASRAR_H_INCLUDED
#include <glm/glm.hpp>

#define NODE_MAX 1000

class Astar
{
public:
	struct NODE
	{
		int x;
		int z;
		int cost;
		struct NODE *parent;	 // êeÉmÅ[Éh
	};
	typedef struct NODE NODE;

	struct LIST
	{
		NODE *node[NODE_MAX];
		int index;
	};
	typedef struct LIST LIST;

	NODE *CreateNode(int x, int z, int cost);
	int G(NODE *s, NODE *n);
	int H(NODE *e, NODE *n);
	void SearchNode(LIST *open, LIST *close, NODE *s,
		NODE *e, NODE *n, NODE *m);
	
	NODE* GetMinCost(NODE *n);
	glm::vec3 CreateRoute(NODE *n);

	NODE s = { 0, 0, 0 };
	NODE e = { 0, 0, 0 };
	LIST open;
	LIST close;

	bool costError = false;

private:

};


#endif // !ASTAR_H_INCLUDED
