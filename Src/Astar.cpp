#include"Astar.h"
#include<stdio.h>

#define ARRAY_NUM(a) (sizeof(a)/sizeof(a[0]))
#define NODE_MAX 1000

/*
次のノードを作成
*/
Astar::NODE *Astar::CreateNode(int x, int z, int cost)
{
	static Astar::NODE n[NODE_MAX];
	static int index = 0;
	n[index].x = x;
	n[index].z = z;
	n[index].cost = cost;
	return &n[index++];
}

/*
スタートノードからnまでの最小コストを返す
*/
int Astar::G(NODE *s, NODE *n)
{
	return n->cost;
}

/*
ゴールノードからnまでの最小コストを返す
*/
int Astar::H(NODE *e, NODE *n)
{
	return 0;
}

/*
*ノードを調べる
*
*@param open  ⇒  計算中のノードを格納しておくためのオープンリスト
*@param close  ⇒  計算済みのノードを格納するためのクローズリスト
*@param s  ⇒  スタートノード
*@param e  ⇒  ゴールノード
*@param n  ⇒  現在のノード
*@param m  ⇒  nのノードに隣接しているノード
*/
void Astar::SearchNode(LIST *open, LIST *close, NODE *s,
	NODE *e, NODE *n, NODE *m)
{
	int inOpen = -1; // openリスト用のチェック変数
	int inClose = -1; // closeリスト用のチェック変数
	int i; // for文用の変数
	// G + H + (nからmまで移動するコスト)
	int fdmCost = G(s, n) + H(e, m) + 1;
	// G + H
	int fsmCost = G(s, m) + H(e, m);

	// mがopenリストに含まれているか
	for (i = 0; i < open->index; i++)
	{
		if (open->node[i] != NULL &&
			m->x == open->node[i]->x &&
			m->z == open->node[i]->z)
		{
			inOpen = i;
			break;
		}
	}

	// mがcloseリストに含まれているか
	for (i = 0; i < close->index; i++)
	{
		if (close->node[i] != NULL &&
			m->x == close->node[i]->x &&
			m->z == close->node[i]->z)
		{
			inClose = i;
			break;
		}
	}

	// mがopenリストにもcloseリストにも含まれていない場合
	if (inOpen == -1 && inClose == -1)
	{
		m->parent = n; // mの親をnとして格納
		open->node[open->index++] = m; // mをopenリストに追加
	}

	// mがopenリストに含まれている場合
	if (inOpen > -1)
	{
		if (fdmCost < fsmCost)
		{
			m->parent = n; // mの親をnとして格納
		}
	}

	// mがcloseリストに含まれている場合
	if (inClose > -1)
	{
		if (fdmCost < fsmCost)
		{
			m->parent = n; // mの親をnとして格納
			open->node[open->index++] = m; // mをopenリストに追加
			close->node[inClose] = NULL; // closeリストを空にする
		}
	}
}

Astar::NODE* Astar::GetMinCost(Astar::NODE *n)
{
	for (int x = 0; x < open.index; x++)
	{
		if (open.node[x] != NULL)
		{
			int cost = open.node[x]->cost;
			if (n == NULL || n->cost > cost)
			{
				// ノードの中で一番最小のコストを得る
				n = open.node[x];
				open.node[x] = NULL;
			}
		}
	}
	return n;
}

glm::vec3  Astar::CreateRoute(Astar::NODE *n)
{
	glm::vec3 p;
	costError = false;
	if (n->cost >= n->parent->cost)
	{
		n = n->parent;
		p = glm::vec3(n->x, 0, n->z);
	}
	// 親ノードのコストが大きかった場合の例外処理
	else if (n->cost < n->parent->cost)
	{
		printf("コストエラー\n");
		// 親ノードの空にする
		n->parent = NULL;
		costError = true;
	}
	return p;
}