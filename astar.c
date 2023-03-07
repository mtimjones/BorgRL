// astar.c -- A* Implementation

#include "headers.h"
#include "ces.h"
#include "logapi.h"
#include <math.h>

struct node_s;

typedef struct node_s
{
   struct node_s *parent;
   int g;
   int h;
   int f;
   int row;
   int col;
} node_t;

#define MAX_LIST     1000
#define MAX_G          50

typedef struct {
   int numElems;
   node_t *elem[ MAX_LIST ];
} list_t;

node_t *start_node_p;

list_t openList_p;
list_t closedList_p;

void listInit( list_t *list_p )
{
   int i;

   list_p->numElems = 0;

   for ( i = 0 ; i < MAX_LIST ; i++ )
   {
      list_p->elem[ i ] = ( node_t * ) NULL;
   }

   return;
}

int listEmpty( list_t *list_p )
{
   if ( list_p->numElems == 0 ) return 1;
   else return 0;
}

node_t *listFindBest( list_t *list_p )
{
   int i;
   int best = -1;

   for ( i = 0 ; i < MAX_LIST ; i++ )
   {
      if ( list_p->elem[i] ) 
      {
         best = i++;
         break;
      }
   }

   for ( ; i < MAX_LIST ; i++ )
   {
      if ( list_p->elem[i] )
      {
         if ( list_p->elem[i]->f < list_p->elem[best]->f )
         {
            best = i;
         }
         else if ( list_p->elem[i]->f == list_p->elem[best]->f )
         {
            // If there are multiple best, chose one at random.
            if ( getSRand( ) < 0.5 ) best = i;
         }
      }
   }

   return list_p->elem[ best ];
}

int listPresent( list_t *list_p, int row, int col )
{
   int i;

   for ( i = 0 ; i < MAX_LIST ; i++ )
   {
      if ( list_p->elem[i] )
      {
         if ( ( list_p->elem[i]->row == row ) && ( list_p->elem[i]->col == col ) )
         {
            return 1;
         }
      }
   }

   return 0;
}

void listAdd( list_t *list_p, node_t *elem_p )
{
   int i;

   for ( i = 0 ; i < MAX_LIST ; i++ ) 
   {
      if ( list_p->elem[i] == ( node_t *)NULL )
      {
         list_p->elem[i] = elem_p;
         list_p->numElems++;
         break;
      }
   }

   return;
}

node_t *listGet( list_t *list_p, int row, int col, int remove )
{
   int i;
   node_t *node = ( node_t * )NULL;

   for ( i = 0 ; i < MAX_LIST ; i++ )
   {
      if ( list_p->elem[i] )
      {
         if ( ( list_p->elem[i]->row == row ) && ( list_p->elem[i]->col == col ) )
         {
            node = list_p->elem[i];
            if ( remove )
            {
               list_p->elem[i] = (node_t *)NULL;
               list_p->numElems--;
            }
            break;
         }
      }
   }

   return node;
}

node_t *allocateNode( int row, int col )
{
   node_t *node_p;

   node_p = malloc( sizeof( node_t ) );

   node_p->parent = ( node_t *) NULL;
   node_p->g = node_p->h = node_p->f = 0.0;
   node_p->row = row;
   node_p->col = col;

   return node_p;
}

const struct {
   int col;
   int row;
} succ[4] = { { 0, -1}, { 0, 1 }, { 1, 0 }, { -1, 0 } };

node_t *getNeighborNode( node_t *curNode_p, int i, int goal_col, int goal_row )
{
   node_t *successor_p = ( node_t *)NULL;
   int row, col;

   row = curNode_p->row + succ[i].row;
   col = curNode_p->col + succ[i].col;

   if ( ( is_cell_empty( col, row ) ) || ( ( goal_col == col ) && ( goal_row == row ) ) )
//   if ( passable( col, row ) )
   {
      successor_p = allocateNode( row, col );
   }

   return successor_p;
}

int calc_h( node_t *node_p, int goal_row, int goal_col )
{
   // Heuristic based upon the Manhattan distance.
   return ( abs(node_p->row-goal_row) + abs(node_p->col-goal_col) );
}

void cleanup( void )
{
   int i;

   for ( i = 0 ; i < MAX_LIST ; i++ )
   {
      if ( openList_p.elem[ i ] ) free( openList_p.elem[ i ] );
      if ( closedList_p.elem[ i ] ) free( closedList_p.elem[ i ] );
   }

   return;
}

void getBestNextStep( node_t *walker, int *row, int *col )
{
   // Note, this works instead of backtrack because we get the path from goal to object.
   // So the first step in the path is the first step we should take.

   walker = ( node_t * )walker->parent;

   if ( walker )
   {
      *row = walker->row, *col = walker->col;
   } 
   else
   {
      *row = *col = 0;
   }

   return;
}


bool MoveTowardsTarget( int start_row, int start_col, int goal_row, int goal_col, int *row, int *col )
{
   node_t *current;

   *row = *col = 0;

   // Initialize the open (frontier) and closed lists.
   listInit( &openList_p );
   listInit( &closedList_p );

   // Allocate our initial node (from the starting point) and add to the open list
   start_node_p = allocateNode( start_row, start_col );
   start_node_p->g = 0;
   start_node_p->h = 0;
   start_node_p->f = calc_h( start_node_p, goal_row, goal_col );
   listAdd( &openList_p, start_node_p );

   while ( !listEmpty(&openList_p) )
   {
      // Find the best node on the frontier
      current = listFindBest( &openList_p );

      // Remove it from the open list
      (void)listGet( &openList_p, current->row, current->col, 1 );

      // Push it to the closed list
      listAdd( &closedList_p, current );

      // Have we reached the goal?
      if ((current->row == goal_row) && (current->col == goal_col)) 
      {
         getBestNextStep( current, row, col );

         if ( *row == 0 && *col == 0 )
         {
            // Nowhere to go, return false to stop.
            cleanup( );
            return false;
         }

         cleanup();

         return true;

      } else {

         // Find each of the four conway neighbors
         for ( int i = 0 ; i < 4 ; i++ ) 
         {
            node_t *neighbor;
            node_t *stored_neighbor;

            neighbor = getNeighborNode( current, i, goal_col, goal_row );

            // If this position is not legal, skip it.
            if ( !neighbor ) continue;

            // If this node is on the closed list, ignore it and move on.
            if ( listPresent( &closedList_p, neighbor->row, neighbor->col ) )
            {
               free( neighbor );
               continue;
            }

            // Calculate this neighbors gscore
            neighbor->parent = current;
            neighbor->g = current->g + 1;
            neighbor->h = calc_h( neighbor, goal_row, goal_col );
            neighbor->f = neighbor->g + neighbor->h;

            // If we're searching too far, just terminate early.
            if ( neighbor->g > MAX_G )
            {
               free( neighbor );
               cleanup( );
               *row = *col = 0;
               return false;
            }

            // If this node doesn't exist...
            stored_neighbor = listGet( &openList_p, neighbor->row, neighbor->col, 0 );
            if ( stored_neighbor == ( node_t * ) 0 )
            {
                // Add the new node here which is a new path.
                listAdd( &openList_p, neighbor );
            }
            else
            {
                // If the score is less, update the open list.
                if ( neighbor->g < stored_neighbor->g )
                {
                    // This node is better, replace the stored node.
                    stored_neighbor->parent = neighbor->parent;
                    stored_neighbor->g = neighbor->g;
                    stored_neighbor->h = neighbor->h;
                    stored_neighbor->f = neighbor->f;
                }
                free( neighbor );
            }
         }
      }
   }

   // We failed to find a solution.
   cleanup();

   *row = *col = 0;

   return false;
}

