/*
 This program is free software; you can redistribute it and/or modify it under
 the terms of the European Union Public Licence - EUPL v.1.1 as published by
 the European Commission.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the European Union Public Licence - EUPL v.1.1
 for more details.

 You should have received a copy of the European Union Public Licence - EUPL v.1.1
 along with this program.

 Further information about the European Union Public Licence - EUPL v.1.1 can
 also be found on the world wide web at http://ec.europa.eu/idabc/eupl

*/

/*
 ------ Copyright (C) 2010 STA Steering Board (space.trajectory.analysis AT gmail.com) ----
*/

/*
 ------------------ Author: Valentino Zuccarelli  -------------------------------------------------
 ------------------ E-mail: (Valentino.Zuccarelli@gmail.com) ----------------------------
 */


#include "Scenario/scenario.h"

double getGravParam_user (double mi_1, double mi_2);
int getSystem (int id_1, int id_2);
#if OLDSCENARIO
ScenarioBody* getBody1 (int System);
ScenarioBody* getBody2 (int System);
#endif
void radiusss (double mi, double x, double y, double z, double &R1, double &R2);

void Jacobi (double mi, double x, double y, double z, double vx, double vy, double vz, double &C);

