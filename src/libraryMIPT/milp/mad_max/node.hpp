#pragma once

#include "types.hpp"

struct BestIntegerSolution;
struct CommonParams_MADmax;

class Node {
public:

    Node(CommonParams_MADmax*, VectorXi32, VectorXi32, BestIntegerSolution*, bool, int, Node* parentNode=nullptr);
    ~Node();

    double LowerBound;
    Node* LowerChildNode;
    Node* UpperChildNode;
    Node* ParentNode;

    VectorXi32 LowerNuBound; //lower bounds on integer variables
    VectorXi32 UpperNuBound; //upper bounds on integer variables

    uint BifurcationIndex; //index of integer variable according to which child nodes are created
    uint BifurcationValueLow; //lower integer value of branching variable
    uint BifurcationValueHigh; //higher integer value of branching variable

    bool isNodeOpen;//determines whether the node has to be further investigated

    BestIntegerSolution* bestSolution;
    CommonParams_MADmax* commonParameters; 

    bool isRootNode; // if true then this is the root node
    bool isLeaf = true; //if true then there are no child nodes

    Eigen::VectorXd nuRelax; //vector of integer variables obtained from LP relaxation, entries may be non-integer
    double roundValue; //value of integer point obtained by rounding procedure

    int depth; //depth of node in the tree, the root node has depth 0

    Eigen::MatrixXd simplexTableau; //simplex tableau of the optimal solution of the LP relaxation
    RowVectorXi32 basicSet; //basic set of simplex tableau

    std::vector<int> non_determined_nu; // index set of nu_i which are not fixed by lower and upper bounds
    std::vector<int> determined_nu;
    std::vector<int> fracIndices;
};

    //bifurcates by creating child nodes uses the previously computes 
    //index and values of the branching variable
    //gives back the number of integer values
    // in the vector of integer variables
int bifurcate(Node*);

    //if values (LowerBound) of child nodes change, they call this update function of
    //their parent node
    //it is also called by the bifurcate method when the child nodes are created
    //updates the lower bound and the open/closed status of the node
    //since the method is called, it is assumed that the child nodes
    //exist
void updateNodeValue(Node*);

    //is called if the best integer solution has improved
    //this affects all nodes, because their values can become worse than
    //the new upper bound
    //in this case the node is closed and its child nodes can be deleted
    //function is called only for the root node, the call then
    //propagates downwards the whole tree
void updateByUpperBound(Node*);

    //returns the leaf node with the minimal value attached to the input
    //node
    //function operates recursively
    //uses the fact that the value of the parent node is always the
    //minimum of the child node values
    //hence we just descend the minimal branch
Node* findMin(Node*);