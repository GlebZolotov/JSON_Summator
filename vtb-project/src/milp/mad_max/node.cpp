#include "node.hpp"

#include <iostream>
#include <Eigen/Dense>

#include "best_solution.hpp"
#include "MADmaximize.hpp"
#include "MADmax_integer_local_improve.hpp"
#include "params.hpp"
#include "utils.hpp"

#include <type_traits>

using namespace Utils;

bool isInfeasible(VectorXi32& nu_lower, VectorXi32& nu_upper) {
       return ((nu_upper-nu_lower).minCoeff() < 0);
}

// presolve for MAD max problem
// tightens the box bounds for the MAD max problem
// uses the two linear inequality constraints with non-integer coefficients
// the coefficients are taken from the commonParameters object
// the bounds are integer vectors
// tightening is repeated until no further progress is achieved
void tighten_boundsMADmax(CommonParams_MADmax* commonParameters, VectorXi32& nu_lower, VectorXi32& nu_upper) {
       VectorXd nu_lower_d = nu_lower.cast<double>();
       VectorXd nu_upper_d = nu_upper.cast<double>();
       VectorXd nu_lower_old = nu_lower_d;
       VectorXd nu_upper_old = nu_upper_d;
       while(1) {
              //tightening upper and lower bound by using linear constraints
              //on Gr'*nu
              VectorXd gnhigh_delta = Utils::floor(rdivide<double>(1-nu_lower_d.dot(commonParameters->params->Gr), 
                                   commonParameters->params->Gr));
              VectorXd gnlow_delta = Utils::floor(rdivide<double>(nu_upper_d.dot(commonParameters->params->Gr) - commonParameters->params->w_min,
                                   commonParameters->params->Gr));
              nu_upper_d = nu_upper_d.cwiseMin(nu_lower_d + gnhigh_delta); 
              nu_lower_d = nu_lower_d.cwiseMax(nu_upper_d - gnlow_delta);
              if((nu_upper_old-nu_upper_d + nu_lower_d-nu_lower_old).sum() == 0 || (nu_upper_d-nu_lower_d).minCoeff() < 0)
                     break;
              nu_lower_old = nu_lower_d;
              nu_upper_old = nu_upper_d;
       }
       nu_lower = nu_lower_d.cast<int32_t>();
       nu_upper = nu_upper_d.cast<int32_t>();
}

/*      % CONSTRUCTOR
       % creates a new node
       % if the argument parentNode is empty, this is the root node
       % when it is created, the node solves its LP relaxation
       % if the value is above the value of the current best solution, the
       % node declares itself closed
       % if the LP solution is integer, the the node is also closed
       % if the integer solution is better than the current best, it
       % replaces the best solution
       % if the value is lower than the current best and not integer, the
       % node declares itself as open and computes the index of the
       % variable for branching */
Node::Node(CommonParams_MADmax* commonParameters, VectorXi32 nu_lower, VectorXi32  nu_upper, 
           BestIntegerSolution* best_solution, bool is_root_node, int nodeDepth, Node* parentNode) 
{
       this->bestSolution = best_solution;
       this->commonParameters = commonParameters;
       this->isRootNode = is_root_node;
       this->depth = nodeDepth;

       if(!is_root_node) {
              this->ParentNode = parentNode;
              tighten_boundsMADmax(commonParameters, nu_lower, nu_upper);
              if(isInfeasible(nu_lower, nu_upper)) {//infeasible case
                     std::clog << "Leaf: Infeasible 1\n";
                     this->LowerBound = double_inf;
                     this->isNodeOpen = false;
                     this->roundValue = double_inf;
                     return;
              } else if((nu_upper-nu_lower).cwiseAbs().sum() == 0) { //only one integer point is potentially feasible
                     VectorXd nu = nu_lower.cast<double>();
                     double MADvar = computeMADvariance(commonParameters, nu);
                     double wGr = commonParameters->params->Gr.dot(nu);
                     if (MADvar <= commonParameters->gamma_mad && commonParameters->params->w_min <= wGr && wGr <= 1) { //point feasible
                            double value = commonParameters->params->rbar*nu;
                            this->isNodeOpen = false;
                            this->roundValue = -value;
                            this->LowerBound = -value;
                            best_solution->setValues(-value, nu_lower); //best_solution->setValues(-value, nu);
                            return;
                     } else { //point not feasible
                            std::clog << "Leaf: Infeasible 2\n";
                            this->LowerBound = double_inf;
                            this->isNodeOpen = false;
                            this->roundValue = double_inf;
                     }
              }
       }

       for(int i = 0; i < nu_lower.size(); ++i) {
              if(nu_lower(i) == nu_upper(i)) 
                    this->determined_nu.push_back(i);
              else {
                     this->non_determined_nu.push_back(i);
              }
       }
       this->LowerNuBound = nu_lower;
       this->UpperNuBound = nu_upper;
       
       //solve LP relaxation
       ReturnValues_MADmaximize returnValCube = MADmaximize_return_on_cube(commonParameters, nu_lower, nu_upper, this);
       //save partial lower bound associated to the node
       double value = returnValCube.value;
       if(!returnValCube.s) {
              this->LowerBound = double_inf;
       } else {
              value = -value;
              this->LowerBound = value;
              this->nuRelax = returnValCube.nu;
              this->simplexTableau = returnValCube.Tab;
              this->basicSet = returnValCube.Bas;
              VectorXd nu_non_det = returnValCube.nu(this->non_determined_nu);
              for(int i = 0; i < nu_non_det.size(); i++) {
                     if(std::abs(nu_non_det(i) - std::round(nu_non_det(i))) > 1e-9) 
                            this->fracIndices.push_back(i);
              }

       }
       //checks if node has higher value than the current best integer one
       if(!returnValCube.s || best_solution->upperBound <= value) {
              this->isNodeOpen = false;
              this->roundValue = double_inf;
       } else {
           //checks if node yields an integer solution
           //look for non-integer value in the solution with the highest
           //sensitivity of the cost function    
           if(returnValCube.single_index && returnValCube.index == 0) {
                  //obtained integer solution, it is better than the best solution
                  this->isNodeOpen = false;
                  this->roundValue = value;
                  best_solution->setValues(value, returnValCube.nu.cast<int32_t>());
           } else {
                     //obtained non-integer solution is better than current integer value
                     //check three integer points
                     ReturnValue_localImprove returnVal_low = MADmax_integer_local_improve(commonParameters, returnValCube.nu_low);
                     value = returnVal_low.value;
                     if (returnVal_low.s) {
                            this->roundValue = -value;
                            if(-value < best_solution->upperBound)
                                   best_solution->setValues(-value, returnVal_low.nu_local);
                     }

                     ReturnValue_localImprove returnVal_upp = MADmax_integer_local_improve(commonParameters, returnValCube.nu_upp);
                     value = returnVal_upp.value;
                     if (returnVal_upp.s) {
                            if(-value < this->roundValue)
                                   this->roundValue = -value;
                            if (-value < best_solution->upperBound)
                                   best_solution->setValues(-value, returnVal_upp.nu_local);
                     }

                     if(!returnValCube.single_index) {
                            //round(nu) may be different from nu_low and nu_upp
                            ReturnValue_localImprove returnVal = MADmax_integer_local_improve(commonParameters, Utils::round(returnValCube.nu).cast<int32_t>());
                            value = returnVal.value;
                            if (returnVal.s) {
                                   if(-value < this->roundValue)
                                          this->roundValue = -value;
                                   if(-value < best_solution->upperBound)
                                          best_solution->setValues(-value, returnVal.nu_local);
                            }
                     }

                     this->isNodeOpen = true;
              }
       }
}

Node::~Node() {
       if(!this->isLeaf) {
              delete this->LowerChildNode;
              delete this->UpperChildNode;
       }
}

void setSensitive(MatrixXd& sensitive, MatrixXd& tab, Node* node, int k, int n=0, int c=0) {
       int bas_ind = Utils::findFirst<int32_t>(node->basicSet, 
              [&](int32_t left) {return left == n + node->fracIndices[k];});
       std::vector<int> negind;
       for(int i = 0; i < tab.cols()-1; i++){
              if(tab.row(bas_ind+1)(i) < 0)
                     negind.push_back(i);
       }
       if(negind.size() != 0) {
              sensitive(k, c) = ((-1)*tab(0, negind).cwiseQuotient(tab(bas_ind+1, negind))).minCoeff();
       } else {
              sensitive(k, c) = double_inf;
       }
}

int bifurcate(Node* node) {
       std::vector<int> ind_relax;
       for(int i=0; i<node->nuRelax.size(); ++i) {
              if(node->nuRelax(i) == std::round(node->nuRelax(i)))
                     ind_relax.push_back(i);
       }
       int num_integer_values = ind_relax.size();
       auto& tab = node->simplexTableau;

       if(tab.size() != 0) {
              MatrixXd sensitive = MatrixXd::Zero(node->fracIndices.size(), 2);
              int n = node->non_determined_nu.size();
              for(int k = 0; k < node->fracIndices.size(); ++k) {
                     setSensitive(sensitive, tab, node, k, 0, 0);
                     setSensitive(sensitive, tab, node, k, n, 1);
              }
              std::vector<int> non_det_nu_fraq;
              for(auto& f : node->fracIndices) {
                     non_det_nu_fraq.push_back(node->non_determined_nu[f]);
              }
              VectorXd relax = node->nuRelax(non_det_nu_fraq);
              VectorXd lower_gains = sensitive(all, 1).cwiseProduct(relax - Utils::floor<double>(relax));
              VectorXd upper_gains = sensitive(all, 0).cwiseProduct(Utils::ceil(relax) - relax);
              int maxind;
              lower_gains.cwiseMin(upper_gains).maxCoeff(&maxind);
              node->BifurcationIndex = node->non_determined_nu[node->fracIndices[maxind]];
       } else {
              node->BifurcationIndex = node->non_determined_nu[node->fracIndices[0]];
       }
       double bfV = node->nuRelax(node->BifurcationIndex);
       node->BifurcationValueHigh = ceil(bfV);
       node->BifurcationValueLow = floor(bfV);
       //higher branch
       VectorXi32 nu_lower_high = node->LowerNuBound;
       nu_lower_high(node->BifurcationIndex) = node->BifurcationValueHigh;
       node->UpperChildNode = new Node(node->commonParameters, nu_lower_high, node->UpperNuBound, 
                                   node->bestSolution, false, node->depth+1, node);
       //lower branch
       VectorXi32 nu_upper_low = node->UpperNuBound;
       nu_upper_low(node->BifurcationIndex) = node->BifurcationValueLow;
       node->LowerChildNode = new Node(node->commonParameters, node->LowerNuBound, nu_upper_low, 
                                   node->bestSolution, false, node->depth+1, node);
       node->isLeaf = false;
       node->simplexTableau.resize(0,0);
       node->basicSet.resize(0);
       updateNodeValue(node);

       return num_integer_values;
}

void updateNodeValue(Node* node) {
       double old_lower_bound = node->LowerBound;
       bool old_status = node->isNodeOpen;
       if(!node->isLeaf) {//should be true automatically
              node->LowerBound = std::min(node->LowerChildNode->LowerBound, node->UpperChildNode->LowerBound);
              //if both child nodes are closed, we may delete them and close the
              //parent node the updated node calls the update routine for the parent node
              if (!node->LowerChildNode->isNodeOpen && !node->UpperChildNode->isNodeOpen) {
                     node->isNodeOpen = false;
                     delete node->LowerChildNode;
                     delete node->UpperChildNode;
                     node->isLeaf = true;
              }
       }
       //ask parent node for update only if something has changed
       if (!node->isRootNode && (node->LowerBound != old_lower_bound || node->isNodeOpen != old_status)) 
              updateNodeValue(node->ParentNode);
}

void updateByUpperBound(Node* node) {
       if(node->LowerBound >= node->bestSolution->upperBound) {
              node->isNodeOpen = false;
              if (!node->isLeaf) {
                     delete node->LowerChildNode;
                     delete node->UpperChildNode;
                     node->isLeaf = true;
              } else {
                     node->simplexTableau.resize(0,0);
                     node->basicSet.resize(0);
              }
       } else {
              if(!node->isLeaf) {
                     updateByUpperBound(node->LowerChildNode);
                     updateByUpperBound(node->UpperChildNode);
              }
       }
}

Node* findMin(Node* node) {
       Node* minNode;
       if(node->isLeaf) {
              minNode = node;
       } else if(node->LowerChildNode->LowerBound < node->UpperChildNode->LowerBound) {
              minNode = findMin(node->LowerChildNode);
       } else {
              minNode = findMin(node->UpperChildNode);
       }
       return minNode;
}