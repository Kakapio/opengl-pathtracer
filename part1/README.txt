This program provides the code for creating 3D models using binary operations on boxes. 

Box.h: This represents a simple 3D axis-aligned box.
BoxSet.h: This represents a non-overlapping set of boxes.

Consider a boxSet with a single box A. When another box B is to be added to this box set:
Case 1: A does not overlap with B. In this case, B is added to the set.

Case 2: A overlaps with B. In this case we create A \ B (a set of boxes that represent the subtraction of B from A). Then the new box set is (A \ B) + B where "+" is just adding a box to the set.

Adding a single box to a boxset with multiple boxes:
Consider a box set consisting of boxes A_1,A_2,...A_n. By definition, they do not overlap with each other.

When a box B is to be added to this box set, we apply the above algorithm repeatedly:

Final box set = (A_1 \ B) + (A_2 \ B) + ... + (A_n \ B) + Box

If A_i and B do not overlap, the result is (A_i \ B) = A_i


Subtracting (difference) a box from a box set:

This is actually a subset of adding above. The only difference is that we do not add B to the final box set.

------------

These two operations can be used to create some interesting models. The main function shows one such example, using only the difference operation.