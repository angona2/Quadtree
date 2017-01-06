/**
 * @file quadtree.h
 * Quadtree class definition.
 * @date Spring 2008
 */

#ifndef QUADTREE_H
#define QUADTREE_H

#include "png.h"

/**
 * A tree structure that is used to compress PNG images.
 */
class Quadtree
{
  public:

		//constructors for Quadtree
		Quadtree();
		Quadtree(PNG const & source, int resoltuion);

		//the big three: Copy constructor, destructor, and overridden '=' operator
		Quadtree(Quadtree const & other);
		~Quadtree();
		Quadtree const & operator=(Quadtree const & other);

		//public memeber functions
		void buildTree(PNG const & source, int resolution);
		RGBAPixel getPixel(int x, int y) const;
		PNG decompress() const;
		void clockwiseRotate();
		void prune(int tolerance);
		int pruneSize(int tolerance) const;
		int idealPrune(int numLeaves) const;

  private:
    /**
     * A simple class representing a single node of a Quadtree.
     * You may want to add to this class; in particular, it could
     * probably use a constructor or two...
     */

		class QuadtreeNode
		{
		  public:
			QuadtreeNode* nwChild; /**< pointer to northwest child */
		    QuadtreeNode* neChild; /**< pointer to northeast child */
		    QuadtreeNode* swChild; /**< pointer to southwest child */
		    QuadtreeNode* seChild; /**< pointer to southeast child */

		    RGBAPixel element; /**< the pixel stored as this node's "data" */

			//added QuadtreeNode attributes of x, y, and resolution
			int x;
			int y;
			int resolution;

			//QuadtreeNode constructor to help build quadtree and store x, y, and resolution easily
			QuadtreeNode(int xpoint, int ypoint, int res){
				x = xpoint;
				y = ypoint;
				resolution = res;

				nwChild = NULL;
				neChild = NULL;
				swChild = NULL;
				seChild = NULL;

			}

			//QuadtreeNode constructor to help our copy function: stores RGBA pixel, resolution, x points, and y points
			QuadtreeNode(const RGBAPixel & ele, int res, int xpoint, int ypoint){
				x = xpoint;
				y = ypoint;
				element = ele;
				resolution = res;

				nwChild = NULL;
				neChild = NULL;
				swChild = NULL;
				seChild = NULL;
			}
		};

		/**< pointer to root of quadtree */
		QuadtreeNode* root;

		//helper function for Buildtree
		void buildTree(PNG const & source, int resolution, QuadtreeNode * root); //takes PNG, resolution, and QuadtreeNode

		//getPixel helper functions
		RGBAPixel getPixel(int x, int y, QuadtreeNode * root) const; //takes x point, y point, and QuadtreeNode (returns RGBApixel)
		bool range(int x, int y, QuadtreeNode * root) const;		 //takes x point, ypoint, and quadtreeNode (returns true if in range or false if not in range)

		//decompres helper function
		void decompress(QuadtreeNode * root, PNG &retval) const; //takes QuadtreeNode and PNG by reference (PNG instantiated in public function based on resolution)

		//clockwiseRotate helper function
		void clockwiseRotate(QuadtreeNode * root); //takes QuadtreeNode

		//prune helper functions
		void prune(QuadtreeNode * root, int tolerance); //takes QuadtreeNode and tolerance
		bool checkTolerance(QuadtreeNode * root, QuadtreeNode * other, int tolerance) const; //takes QuadtreeNode, QuadtreeNode, and tolerance (returns true or false (difference <= tolerance))

		//pruneSize helper functions
		int pruneSize(QuadtreeNode * root, int tolerance) const; //takes QuadtreeNode and tolerance (returns amount of leaves pruned with a given tolerance)

		//ideaPrune helper function
		int idealPrune(int lowerbound, int upperBound, int numLeaves) const; //takes lowerbound (0), upperbound (255 * 255 * 3), and a number of leaves (returns tolerance needed to produce a tree with 'numLeaves' remaining)

		//Big Three helpers
		void copy(const Quadtree & other); //takes another Quadtree and copies it into current tree
		void copy(QuadtreeNode * root, QuadtreeNode * other, int resolution); //copy helper function: does the actual calculations and copies new tree in
		void clear(QuadtreeNode *& root); //deallocates all data in our destructor and utilized to 'prune' children nodes

/**** Functions for testing/grading                      ****/
/**** Do not remove this line or copy its contents here! ****/
#include "quadtree_given.h"
};

#endif
