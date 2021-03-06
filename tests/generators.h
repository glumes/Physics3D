#pragma once

#include <utility>

#include "../physics/math/linalg/vec.h"
#include "../physics/math/linalg/mat.h"
#include "../physics/math/rotation.h"
#include "../physics/math/cframe.h"
#include "../physics/math/globalCFrame.h"
#include "../physics/math/position.h"
#include "../physics/math/bounds.h"
#include "../physics/math/fix.h"
#include "../physics/math/taylorExpansion.h"
#include "../physics/geometry/shape.h"
#include "../physics/geometry/shapeCreation.h"
#include "../physics/geometry/polyhedron.h"
#include "../physics/motion.h"
#include "../physics/relativeMotion.h"

#include "../physics/part.h"
#include "../physics/physical.h"
#include "../physics/constraints/hardConstraint.h"
#include "../physics/layer.h"
#include "../physics/world.h"

#include "../physics/datastructures/boundsTree.h"

int generateInt(int max);
size_t generateSize_t(size_t max);
double generateDouble();
bool generateBool();
Shape generateShape();
Vec3 generateVec3();
Position generatePosition();
Bounds generateBounds();
Rotation generateRotation();
CFrame generateCFrame();
GlobalCFrame generateGlobalCFrame();
template<int Derivatives, typename ItemGenerator>
auto generateTaylor(ItemGenerator itemGenerator) -> TaylorExpansion<decltype(itemGenerator()), Derivatives> {
	TaylorExpansion<decltype(itemGenerator()), Derivatives> result;

	for(auto& item : result) {
		item = itemGenerator();
	}

	return result;
}
template<int Derivatives, typename ItemGenerator>
auto generateFullTaylor(ItemGenerator itemGenerator) -> FullTaylorExpansion<decltype(itemGenerator()), Derivatives> {
	FullTaylorExpansion<decltype(itemGenerator()), Derivatives> result;

	for(auto& item : result) {
		item = itemGenerator();
	}

	return result;
}
TranslationalMotion generateTranslationalMotion();
RotationalMotion generateRotationalMotion();
Motion generateMotion();
RelativeMotion generateRelativeMotion();
PartProperties generatePartProperties();
Part generatePart();
Part generatePart(Part& attachTo);
Part generatePart(Part& attachTo, HardConstraint* constraint);
HardConstraint* generateHardConstraint();
void generateAttachment(Part& first, Part& second);
std::vector<Part> generateMotorizedPhysicalParts();
void generateLayerAssignment(std::vector<Part>& parts, WorldPrototype& world);
TreeNode generateTreeNode(int branchInhibition);
BoundsTree<BasicBounded> generateFilledBoundsTree();
BoundsTree<BasicBounded> generateBoundsTree();
void* getRandomLeafObject(const TreeNode& node);
template<typename Boundable>
Boundable* getRandomObjectFromTree(const BoundsTree<Boundable>& tree) {
	return static_cast<Boundable*>(getRandomLeafObject(tree.rootNode));
}
template<typename Collection>
auto oneOf(const Collection& collection) -> decltype(collection[0]) {
	return collection[generateSize_t(collection.size())];
}
template<typename Collection>
auto oneOf(Collection& collection) -> decltype(collection[0]) {
	return collection[generateSize_t(collection.size())];
}
