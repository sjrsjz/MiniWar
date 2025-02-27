﻿#include "../../header/utils/Operation.h"

Operation::Operation() {
}

Operation::Operation(Operator op) {
	this->op = op;
}

Operation::Operation(int id, Operator op) {
	this->id = id;
	this->op = op;
}

Operation::Operation(Point cur, Operator op) {
	this->cur = cur;
	this->op = op;
}

Operation::Operation(Point cur, double radius, Operator op) {
	this->cur = cur;
	this->radius = radius;
	this->op = op;
}

Operation::Operation(Point start, Point end, Operator op) {
	this->start = start;
	this->end = end;
	this->op = op;
}

Operation::Operation(Point start, Point end, int size, Operator op) {
	this->start = start;
	this->end = end;
	this->size = size;
	this->op = op;
}

Point Operation::getStart() {
	return start;
}

Point Operation::getEnd() {
	return end;
}

Point Operation::getCur() {
	return cur;
}

Operator Operation::getOp() {
	return op;
}

int Operation::getSize() {
	return size;
}

double Operation::getRadius() {
	return radius;
}

int Operation::getId() {
	return id;
}

