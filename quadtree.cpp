/**
 * @file quadtree.cpp
 * Quadtree class implementation.
 * @date Spring 2008
 */

#include <iostream>
#include "quadtree.h"

using namespace std;


/*
*The no parameters constructor takes no arguments, and produces an empty Quadtree object, i.e.
*one which has no associated QuadtreeNode objects, and in which root is NULL.
*/
Quadtree::Quadtree(){
	root = NULL;
}




/*
*This constructor's purpose is to build a Quadtree representing the upper-left d by d block of the *source image.
*This effectively crops the source image into a d by d square.
*You may assume that d is a power of two, and that the width and height of source are each at least d.
*/
Quadtree::Quadtree(PNG const & source, int resolution){
	root = NULL;
	buildTree(source, resolution);
}




/*
*Copy constructor.
*Simply sets this Quadtree to be a copy of the parameter.
*/
Quadtree::Quadtree(Quadtree const & other){
	if(other.root == NULL){
		root = NULL;
		return;
	}

	copy(other);
}




/*
Destructor; frees all memory associated with this Quadtree.
*/
Quadtree::~Quadtree(){
	clear(root);
}




/*
*Assignment operator; frees memory associated with this Quadtree and sets its contents to be equal to *the parameter's.
*/
Quadtree const & Quadtree::operator=(Quadtree const & other){
	if(this != &other){
		clear(root);
		copy(other);
	}

	return *this;
}




/*
*Deletes the current contents of this Quadtree object, then turns it into a Quadtree object *representing the upper-left d by d block of source.
*You may assume that d is a power of two, and that the width and height of source are each at least d.
*/
void Quadtree::buildTree(PNG const & source, int resolution){
	if(root != NULL){
		clear(root);
	}

	root = new QuadtreeNode(0, 0, resolution);
	buildTree(source, resolution, root);
}

//Buildtree Helper Function
void Quadtree::buildTree(PNG const & source, int resolution, QuadtreeNode * root){
	//base case, once resolution is one we assign elements to nodes
	if(resolution == 1){
		root->element = *(source(root->x, root->y));
		return;
	}

	//creates new children initializing their x, y, and resolution
	root->nwChild = new QuadtreeNode(root->x, root->y, resolution/2);
	root->neChild = new QuadtreeNode(root->x + (resolution/2), root->y, resolution/2);
	root->swChild = new QuadtreeNode(root->x, root->y + (resolution/2), resolution/2);
	root->seChild = new QuadtreeNode(root->x + (resolution/2), root->y + (resolution/2), resolution/2);

	//recursive call to half resolution and call children recursively
	buildTree(source, resolution/2, root->nwChild);
	buildTree(source, resolution/2, root->neChild);
	buildTree(source, resolution/2, root->swChild);
	buildTree(source, resolution/2, root->seChild);

	//sets parent node colors
		root->element.red = (root->nwChild->element.red +
							 root->neChild->element.red +
							 root->swChild->element.red +
							 root->seChild->element.red)/4;
		root->element.blue = (root->nwChild->element.blue +
							  root->neChild->element.blue +
							  root->swChild->element.blue +
							  root->seChild->element.blue)/4;
		root->element.green = (root->nwChild->element.green +
							   root->neChild->element.green +
							   root->swChild->element.green +
							   root->seChild->element.green)/4;


}




/*
*Gets the RGBAPixel corresponding to the pixel at coordinates (x, y) in the bitmap image which the *Quadtree represents.
*Note that the Quadtree may not contain a node specifically corresponding to this pixel (due, for *instance, to pruning - see below). In this case, getPixel will retrieve the pixel (i.e. the color) *of the square region within which the smaller query grid cell would lie. (That is, it will return *the element of the nonexistent leaf's deepest surviving ancestor.) If the supplied coordinates fall *outside of the bounds of the underlying bitmap, or if the current Quadtree is "empty" (i.e., it was *created by the default constructor) then the returned RGBAPixel should be the one which is created *by the default RGBAPixel constructor.
*/
RGBAPixel Quadtree::getPixel(int x, int y) const{
	if(root != NULL && x <= root->resolution && y <= root->resolution){
		return getPixel(x, y, root);
	}

	return RGBAPixel();
}

//getPixel helper function
RGBAPixel Quadtree::getPixel(int x, int y, QuadtreeNode * root) const{
	//base case, resolution reaches 1 it returns element
	if((root->x == x && root->y == y && root->resolution ==1) || root->nwChild == NULL){
		return root->element;
	}

	//if statements to see if x and y are within a specific child
	if(range(x, y, root->nwChild)){
		return getPixel(x, y, root->nwChild);
	}
	else if(range(x, y, root->neChild)){
		return getPixel(x, y, root->neChild);
	}
	else if(range(x, y, root->swChild)){
		return getPixel(x, y, root->swChild);
	}
	else{
		return getPixel(x , y, root->seChild);
	}

}

//getPixel helper function (returns weather or not x and y lies within a specific child passed by previous helper fucntion
bool Quadtree::range(int x, int y, QuadtreeNode * root) const{
	int xr = root->x + root->resolution;
	int yr = root->y + root->resolution;

	return ((x >= root->x && x < xr) && (y >= root->y && y < yr));
}




/*
*Returns the underlying PNG object represented by the Quadtree.
*If the current Quadtree is "empty" (i.e., it was created by the default constructor) then the *returned PNG should be the one which is created by the default PNG constructor. This function *effectively "decompresses" the Quadtree. A Quadtree object, in memory, may take up less space than *the underlying bitmap image, but we cannot simply look at the Quadtree and tell what image it *represents. By converting the Quadtree back into a bitmap image, we lose the compression, but gain *the ability to view the image directly.
*/

PNG Quadtree::decompress() const{
	//create PNG of size resolution by resolution call decompress and return changed value
	if(root != NULL){
		PNG retval(root->resolution, root->resolution);
		decompress(root, retval);
		return retval;
	}

	//returns empty PNG if root is NULL
	return PNG();
}

//decompress helper function, pass PNG by reference and store current root's pixel in PNG using preorder traversal of the quad tree
void Quadtree::decompress(QuadtreeNode * root, PNG &retval) const{
	//returns if root is NULL
	if(root == NULL){
		return;
	}

	//temp variable stores resolution (for loop base case to get x and y values)
	int res = root->resolution;

	//stores pixel in PNG image using getPixel (PNG passed by reference here and returned above)
	for(int x = 0; x < res; x++){
		for(int y = 0; y < res; y++){
			*retval(x,y) = getPixel(x, y);
		}
	}
}




/*
*Rotates the Quadtree object's underlying image clockwise by 90 degrees.
*(Note that this should be done using pointer manipulation, not by attempting to swap the element *fields of QuadtreeNodes. Trust us; it's easier this way.)
*/

void Quadtree::clockwiseRotate(){
	//call helper function if root is not null
	if(root != NULL){
		clockwiseRotate(root);
	}

	//return if root is null
	return;
}

//clockwiseRotate helper function
void Quadtree::clockwiseRotate(QuadtreeNode * root){
	//if nwChild of root = NULL then return
	if(root->nwChild == NULL){
		return;
	}

	//temp stores nwChild
	QuadtreeNode * temp = root->nwChild;

	//pointer manipulation
	root->nwChild = root->swChild;	//nwChild = swChild
	root->swChild = root->seChild;	//swChild = seChild
	root->seChild = root->neChild;	//seChild = neChild
	root->neChild = temp;			//neChild = temp(nwChild)

	//resets x and y values for each child after pointers manipulated to rotate 90 degrees
	root->nwChild->x = root->x;
	root->nwChild->y = root->y;

	root->neChild->x = root->x + root->resolution/2;
	root->neChild->y = root->y;

	root->swChild->x = root->x;
	root->swChild->y = root->y + root->resolution/2;

	root->seChild->x = root->x + root->resolution/2;
	root->seChild->y = root->y + root->resolution/2;

	//recursive call
	clockwiseRotate(root->nwChild);
	clockwiseRotate(root->neChild);
	clockwiseRotate(root->swChild);
	clockwiseRotate(root->seChild);
}




/*
*Compresses the image this Quadtree represents.
*
*If the color values of the leaves of a subquadtree don't vary by much, we might as well represent *the entire subtree by, say, the average color value of those leaves. We may use this information to *effectively "compress" the image, by strategically trimming the Quadtree.
*
*Consider a node n and the subtree, Tn rooted at n, and let avg denote the component-wise average *color value of all the leaves of Tn. Component-wise average means that every internal node in the *tree calculates its value by averaging its immediate children. This implies that the average must be *calculated in a "bottom-up" manner.
*
*
*Due to rounding errors, using the component-wise average is not equivalent to using the true average *of all leaves in a subtree. If a node n is pruned, the children of n and the subtrees for which they *are the roots are removed from the Quadtree. Node n is pruned if the color value of no leaf in Tn, *differs from avg by more than tolerance. (Note: for all average calculations, just truncate the *value to integer.)
*
*We define the "difference" between two colors, (r1,g1,b1) and (r2,g2,b2), to be (r2−r1)2+(g2−g1)2+(b*−b1)2.
*
*To be more complete, if the tolerance condition is met at a node n, then the block of the underlying *image which n represents contains only pixels which are "nearly" the same color. For each such node *n, we remove from the Quadtree all four children of n, and their respective subtrees (an operation *we call a pruning). This means that all of the leaves that were deleted, corresponding to pixels *whose colors were similar, are now replaced by a single leaf with color equal to the average color *over that square region.
*
*The prune function, given a tolerance value, prunes the Quadtree as extensively as possible. (Note, *however, that we do not want the prune function to do an "iterative" prune. It is conceivable that *by pruning some mid-level node n, an ancestor p of n then becomes prunable, due to the fact that the *prune changed the leaves descended from p. Your prune function should evaluate the prunability of *each node based on the presence of all nodes, and then delete the subtrees based at nodes deemed *prunable.)
*
*Note
*You should start pruning from the root of the Quadtree.
*Parameters
*tolerance	The integer tolerance between two nodes that determines whether the subtree can be pruned.
*/

void Quadtree::prune(int tolerance){
	if(root != NULL){
		prune(root, tolerance);
	}

	return;
}

//prune helper function
void Quadtree::prune(QuadtreeNode * root, int tolerance){
	//base case, return when nwChild is null
	if(root->nwChild == NULL){
		return;
	}

	//if children are within tolerance then parents color = children average color and then clears out children and returns
	if(checkTolerance(root, root, tolerance)){

		int red = (root->nwChild->element.red + root->neChild->element.red + root->swChild->element.red + root->seChild->element.red)/4;
		int blue = (root->nwChild->element.blue + root->neChild->element.blue + root->swChild->element.blue + root->seChild->element.blue)/4;
		int green = (root->nwChild->element.green + root->neChild->element.green + root->swChild->element.green + root->seChild->element.green)/4;

		root->element.red = red;
		root->element.blue = blue;
		root->element.green = green;

		clear(root->nwChild);
		clear(root->neChild);
		clear(root->swChild);
		clear(root->seChild);
		return;
	}

	//recursive call to each child
	prune(root->nwChild, tolerance);
	prune(root->neChild, tolerance);
	prune(root->swChild, tolerance);
	prune(root->seChild, tolerance);
}

//prune helper function to see if child lies within tolerance of its parent node
bool Quadtree::checkTolerance(QuadtreeNode * root, QuadtreeNode * other, int tolerance) const{
	//base case, return true or false when nwChild is NULL
	if(other->nwChild == NULL){
		//runs difference algorithm
		int red = ((other->element.red - root->element.red) * (other->element.red - root->element.red));
		int blue = ((other->element.blue - root->element.blue) * (other->element.blue - root->element.blue));
		int green = ((other->element.green - root->element.green) * (other->element.green - root->element.green));
		int dif = (red + green + blue);

		//true if difference is less than or equal to tolerance
		return dif <= tolerance;
	}

	//recursive call, for prune to occur base case needs to be true for all children
	return (checkTolerance(root, other->nwChild, tolerance)&&
			checkTolerance(root, other->neChild, tolerance)&&
			checkTolerance(root, other->swChild, tolerance)&&
			checkTolerance(root, other->seChild, tolerance));
}




/*
*This function is similar to prune; however, it does not actually prune the Quadtree.
*
*Rather, it returns a count of the total number of leaves the Quadtree would have if it were pruned *as in the prune function.
*
*Parameters
*tolerance	The integer tolerance between two nodes that determines whether the subtree can be pruned.
*Returns
*How many leaves this Quadtree would have if it were pruned with the given tolerance.
*/

int Quadtree::pruneSize(int tolerance) const{
	//call helper function if root is not null and tolerance is greater than or equal to 0
	if(root != NULL && tolerance >= 0){
		return pruneSize(root, tolerance);
	}

	//if either condition above fails then return 0
	return 0;
}

//pruneSize helper function
int Quadtree::pruneSize(QuadtreeNode * root, int tolerance) const{
	//base case, if nwChild is null then returns 1
	if(root->nwChild == NULL){
		return 1;
	}

	//base case, if current node is within tolerance then return one
	if(checkTolerance(root, root, tolerance)){
		return 1;
	}

	//adds each call of each child called recursively, this returns number of leaves given a tolerance
	return pruneSize(root->nwChild, tolerance) + pruneSize(root->neChild, tolerance) + pruneSize(root->swChild, tolerance) + pruneSize(root->seChild, tolerance);
}




/*
*Calculates and returns the minimum tolerance necessary to guarantee that upon pruning the tree, no *more than numLeaves leaves remain in the Quadtree.
*
*Essentially, this function is an inverse of pruneSize; for any Quadtree object theTree, and for any *positive integer tolerance it should be true that
*
*theTree.pruneSize(theTree.idealPrune(numLeaves)) <= numLeaves
*Once you understand what this function is supposed to do, you will probably notice that there is an *"obvious" implementation. This is probably not the implementation you want to use! There is a fast *way to implement this function, and a slow way; you will need to find the fast way. (If you doubt *that it makes a significant difference, the tests in the given main.cpp should convince you.)
*
*Parameters
*numLeaves	The number of leaves you want to remain in the tree after prune is called.
*Returns
*The minimum tolerance needed to guarantee that there are no more than numLeaves remaining in the *tree.
*Note
*The "obvious" implementation involves a sort of linear search over all possible tolerances. What if *you tried a binary search instead?
*/

int Quadtree::idealPrune(int numLeaves) const{
	//calls helper function if root is not null
	if(root != NULL){
		return idealPrune(0, 255 * 255 * 3, numLeaves);
	}

	//if root is null returns 0
	return 0;
}

//idealPrune helper function
int Quadtree::idealPrune(int lower, int upper, int numLeaves) const{
	//temp variable representing leaves returned from pruneSize
	int tol;

	//if lowerbound ever goes over the upper bound then return the lower bound
	if(lower > upper){
		return lower;
	}

	//store leaves of lower+upper/2
	tol = pruneSize((lower + upper)/2);

	//if tolerance = number of leaves and if tolerance = number of leaves with 1 less then recursive cal with half of lower+upper - 1;
	if(tol == numLeaves){
		if(tol == pruneSize(((lower + upper)/2)-1)){
			return idealPrune(0, ((lower + upper)/2)-1, numLeaves);
		}

		//if the num leaves from 'tol' doesnt statisfy then return lower+upper/2
		return (lower+upper)/2;
	}

	//if number of leaves produced by given tolerance is > numLeaves then divide lower+upper and add 1 to value for the lower bound
	else if(tol > numLeaves){
		return idealPrune(((lower + upper)/2)+1, upper, numLeaves);
	}

	//if number of leaves produced by given tolerance is < numLeaves then divide lower+upper subtract 1 to value for the upper bound
	else if(tol < numLeaves){
		return idealPrune(lower, ((lower+upper)/2)-1, numLeaves);
	}

	//return 0 if no conditions are met
	else{
		return 0;
	}
}




//copy function to assist "Big Three" functions
void Quadtree::copy(const Quadtree & other){
	//checks to make sure we have a tree to even copy first
	if(other.root == NULL){
		root = NULL;
		return;
	}

	//root is copied
	root = new QuadtreeNode(other.root->element, other.root->resolution, other.root->x, other.root->y);

	//if all children are NULL then stop here
	if(other.root->nwChild == NULL && other.root->neChild == NULL && other.root->swChild == NULL && other.root->seChild == NULL){
		return;
	}

	//call helper
	copy(root, other.root, other.root->resolution);
}


//copy helper function
void Quadtree::copy(QuadtreeNode * root, QuadtreeNode * other, int resolution){
	//if all children are not null
	if(other->nwChild == NULL){
		return;
	}
		//set all children of root to new copied values
		root->nwChild = new QuadtreeNode(other->nwChild->element, other->nwChild->resolution, other->nwChild->x, other->nwChild->y);
		root->neChild = new QuadtreeNode(other->neChild->element, other->neChild->resolution, other->neChild->x, other->neChild->y);
		root->swChild = new QuadtreeNode(other->swChild->element, other->swChild->resolution, other->swChild->x, other->swChild->y);
		root->seChild = new QuadtreeNode(other->seChild->element, other->seChild->resolution, other->seChild->x, other->seChild->y);

		//recursive call
		copy(root->nwChild, other->nwChild, resolution/2);
		copy(root->neChild, other->neChild, resolution/2);
		copy(root->swChild, other->swChild, resolution/2);
		copy(root->seChild, other->seChild, resolution/2);
}




//helper function to clear current quadtree
void Quadtree::clear(QuadtreeNode *& root){
	//base case, root is null then return
	if(root == NULL){
		return;
	}
	//recursive call to each child of quadtree's root
	clear(root->nwChild);
	clear(root->neChild);
	clear(root->swChild);
	clear(root->seChild);

	//deletes root and sets to NULL
	delete root;
	root = NULL;
}
