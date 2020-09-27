#include "../../Includes/UserInterface/String.h"

#include <string>

/* Copyright (c) 2020 [Rick de Bondt] - String.cpp */

void String::Draw()
{
    GetWindow().DrawString(GetYCoord(), GetXCoord(), 1, GetName());
}