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
 ------------------ Author:       Guillermo Ortega               -------------------
 ------------------ Affiliation:  European Space Agency (ESA)    -------------------
 -----------------------------------------------------------------------------------

*/

#include "Scenario/staschema.h"
#include "Scenario/MissionsDefaults.h"



MissionsDefaults::~MissionsDefaults()
{

}


/////////////////////////////////////// Generic elements //////////////////////////////////////

void MissionsDefaults::MissionsDefaults_GenericPlatform(ScenarioSCSystemType* genericPlatform)
{
    genericPlatform->SystemBudgets()->MassOfSystem()->setDryMass(1000.0);
    genericPlatform->SystemBudgets()->MassOfSystem()->setWetMass(1500.0);
    genericPlatform->SystemBudgets()->PowerOfSystem()->setTotalPowerBoL(200.0);
    genericPlatform->SystemBudgets()->PowerOfSystem()->setTotalPowerEoL(100.0);
    genericPlatform->SystemBudgets()->Link()->setDownlinkMargin(10);
    genericPlatform->SystemBudgets()->Link()->setUplinkMargin(10);

    genericPlatform->SCAerodynamics()->setSurfaceArea(1.0);
    genericPlatform->SCAerodynamics()->setCd(0.25);

    genericPlatform->Propulsion()->setNumberOfEngines(1);
    genericPlatform->Propulsion()->setPropellantMass(500);
    genericPlatform->Propulsion()->setSpecificImpulse(312);
    genericPlatform->Propulsion()->setThrustPerEngine(100);
    genericPlatform->Propulsion()->setTotalPropulsionDryMass(25);

    genericPlatform->Structure()->setStructuralShape("Cube");			    // Cube, Cylinder or Sphere
    genericPlatform->Structure()->setStructuralMaterial("Aluminium");		    // Steel or Aluminium
    genericPlatform->Structure()->setTotalStructureMass(800);
    genericPlatform->Structure()->Sizing()->setHeight(1);
    genericPlatform->Structure()->Sizing()->setLength(1);
    genericPlatform->Structure()->Sizing()->setWidth(1);
    genericPlatform->Structure()->Sizing()->setVolume(1);
    genericPlatform->Structure()->MomentsOfInertia()->setXAxis(150);
    genericPlatform->Structure()->MomentsOfInertia()->setYAxis(150);
    genericPlatform->Structure()->MomentsOfInertia()->setZAxis(150);
    genericPlatform->Structure()->SecondMomentsOfArea()->setXAxis(20);
    genericPlatform->Structure()->SecondMomentsOfArea()->setYAxis(20);
    genericPlatform->Structure()->SecondMomentsOfArea()->setZAxis(20);
    genericPlatform->Structure()->NaturalFrequency()->setAxialFrequency(50);
    genericPlatform->Structure()->NaturalFrequency()->setLateralFrequency(12);

    genericPlatform->TCS()->Temperature()->setMaximumSCTemperature(85);
    genericPlatform->TCS()->Temperature()->setMinimumSCTemperature(15);
    genericPlatform->TCS()->setHeaterPower(50);
    genericPlatform->TCS()->setRadiatedPower(12);
    genericPlatform->TCS()->setHeaterPower(25);
    genericPlatform->TCS()->setTotalTCSMass(15);
    genericPlatform->TCS()->CoatingArea()->ColdSurface()->ColdCoating()->setAbsorptivity(0.2);
    genericPlatform->TCS()->CoatingArea()->ColdSurface()->ColdCoating()->setEmissivity(0.3);
    genericPlatform->TCS()->CoatingArea()->ColdSurface()->setTotalColdArea(1);
    genericPlatform->TCS()->CoatingArea()->HotSurface()->HotCoating()->setAbsorptivity(0.2);
    genericPlatform->TCS()->CoatingArea()->HotSurface()->HotCoating()->setEmissivity(0.3);
    genericPlatform->TCS()->CoatingArea()->HotSurface()->setTotalHotArea(1);

    genericPlatform->EPS()->SolarArray()->setSACellType("Silicon");		    // Silicon, GaAs, Multijunction
    genericPlatform->EPS()->SolarArray()->setAreaOfSolarArray(4);
    genericPlatform->EPS()->SolarArray()->setMassOfSolarArray(30);
    genericPlatform->EPS()->BatteryType()->setBatteryType("Lithium_Ion");	    // Lithium_Ion, Nickel_Cadmium, Lead_Acid
    genericPlatform->EPS()->BatteryType()->setNumberOfBatteries(4);
    genericPlatform->EPS()->BatteryType()->setTotalMassOfBatteries(30);
    genericPlatform->EPS()->BatteryType()->setTotalVolumeOfBatteries(0.01);
    genericPlatform->EPS()->setMassOfPCU(0.1);
    genericPlatform->EPS()->setTotalEPSMass(70);

    genericPlatform->TTC()->setTotalTTCMass(70);
    genericPlatform->TTC()->TTCAntenna()->setDataRate(300);
    genericPlatform->TTC()->TTCAntenna()->setLinkDuration(10);
    genericPlatform->TTC()->TTCAntenna()->setTransmitterEfficiency(90);
    genericPlatform->TTC()->TTCAntenna()->setTransmitterFrequency(25);
    genericPlatform->TTC()->TTCAntenna()->setTransmitterGain(30);
    genericPlatform->TTC()->TTCAntenna()->setTransmitterPower(100);

    genericPlatform->AOCS()->setTotalAOCSMass(0.5);

    genericPlatform->OBDH()->setTotalOBDHMass(0.5);
    genericPlatform->OBDH()->setTotalSizeOfMemory(200);

}

void MissionsDefaults_GenericTransmitter(ScenarioTransmitterPayloadType* genericTransmitter)
{

}

void MissionsDefaults_GenericReceiver(ScenarioReceiverPayloadType* genericReceiver)
{

}

void MissionsDefaults_GenericOpticalPayload(ScenarioOpticalPayloadType* genericOpticalPayload)
{

}

void MissionsDefaults_GenericRadarPayload(ScenarioRadarPayloadType* genericRadarPayload)
{

}


//////////////////////////////////////// Science /////////////////////////////////////////////////

void MissionsDefaults::MissionsDefaults_XMM(ScenarioLoiteringType* loiteringXMM)
{

}


void MissionsDefaults_INTEGRAL(ScenarioLoiteringType* loiteringINTEGRAL)
{

}

void MissionsDefaults_MEX(ScenarioLoiteringType* loiteringMEX)
{
}

void MissionsDefaults_Rosetta(ScenarioLoiteringType* loiteringRosetta)
{
}

void MissionsDefaults_VEX(ScenarioLoiteringType* loiteringVEX)
{
}

void MissionsDefaults_Herschel(ScenarioLoiteringType* loiteringHerschel)
{
}

void MissionsDefaults_Planck(ScenarioLoiteringType* loiteringPlanck)
{
}

void MissionsDefaults_LISA(ScenarioLoiteringType* loiteringLISA)
{
}

void MissionsDefaults_BepiColombo(ScenarioLoiteringType* loiteringBepiColombo)
{
}

void MissionsDefaults_ExoMars(ScenarioLoiteringType* loiteringExoMars)
{
}


//////////////////////////////////////// Observation of the Earth /////////////////////////////
void MissionsDefaults_Meteosat6(ScenarioLoiteringType* loiteringMeteosat6)
{
}

void MissionsDefaults_ERS2(ScenarioLoiteringType* loiteringERS2)
{
}

void MissionsDefaults_ENVISAT(ScenarioLoiteringType* loiteringENVISAT)
{
}

void MissionsDefaults_MSG(ScenarioLoiteringType* loiteringMSG)
{
}

void MissionsDefaults_MTG(ScenarioLoiteringType* loiteringMTG)
{
}

void MissionsDefaults_METOP(ScenarioLoiteringType* loiteringMETOP)
{
}

void MissionsDefaults_Cryosat(ScenarioLoiteringType* loiteringCryosat)
{
}

void MissionsDefaults_GOCE(ScenarioLoiteringType* loiteringGOCE)
{
}

void MissionsDefaults_SMOS(ScenarioLoiteringType* loiteringSMOS)
{
}

void MissionsDefaults_Aeolus(ScenarioLoiteringType* loiteringAeolus)
{
}

void MissionsDefaults_EarthCare(ScenarioLoiteringType* loiteringEarthCare)
{
}

void MissionsDefaults_Sentinel1(ScenarioLoiteringType* loiteringSentine1)
{
}

void MissionsDefaults_Sentinel2(ScenarioLoiteringType* loiteringSentine2)
{
}

void MissionsDefaults_Sentinel3(ScenarioLoiteringType* loiteringSentine3)
{
}

void MissionsDefaults_Sentinel5(ScenarioLoiteringType* loiteringSentine5)
{
}


////////////////////////////////////// Tecommunications //////////////////////////////
void MissionsDefaults_Artemis(ScenarioLoiteringType* loiteringArtemis)
{
}

void MissionsDefaults_Alphasat(ScenarioLoiteringType* loiteringAlphasat)
{
}

void MissionsDefaults_SmallGEO(ScenarioLoiteringType* loiteringSmallGEO)
{
}


// TEC
void MissionsDefaults_PROBA1(ScenarioLoiteringType* loiteringPROBA1)
{
}

void MissionsDefaults_PROBA2(ScenarioLoiteringType* loiteringPROBA2)
{
}

void MissionsDefaults_PROBA3(ScenarioLoiteringType* loiteringPROBA3)
{
}


///////////////////////////////////////////// Human space flight ////////////////////////////////////////
 ScenarioLoiteringType MissionsDefaults::MissionsDefaults_ISS()
{

     ScenarioLoiteringType loitering;

    /*** fill in defaults ***/
    loitering.Environment()->CentralBody()->setName("Earth");
    loitering.InitialPosition()->setCoordinateSystem("INERTIAL J2000");
    loitering.InitialAttitude()->setCoordinateSystem("EULER 123");

    QSharedPointer<ScenarioKeplerianElementsType> initPos(new ScenarioKeplerianElementsType());
    initPos->setSemiMajorAxis(6378.0 + 450.1365);
    initPos->setInclination(51.6651);
    initPos->setEccentricity(0.0006);
    initPos->setRAAN(12.4829);
    initPos->setArgumentOfPeriapsis(45.0278);
    initPos->setTrueAnomaly(87.3523);
    loitering.InitialPosition()->setAbstract6DOFPosition(initPos);
    QSharedPointer<ScenarioEulerBIType> initAtt(new ScenarioEulerBIType());
    initAtt->setPhi(0.00000);
    initAtt->setTheta(0.00000);
    initAtt->setPsi(0.00000);
    initAtt->setPhiDot(0.00000);
    initAtt->setThetaDot(0.00000);
    initAtt->setPsiDot(0.00000);
    loitering.InitialAttitude()->setAbstract6DOFAttitude(initAtt);

    QDateTime TheCurrentDateAndTime = QDateTime::currentDateTime(); // Get the current epoch
    loitering.TimeLine()->setStartTime(TheCurrentDateAndTime);
    loitering.TimeLine()->setEndTime(TheCurrentDateAndTime.addDays(1));
    loitering.TimeLine()->setStepTime(60.0);

    loitering.PropagationPosition()->setPropagator("TWO BODY");
    loitering.PropagationPosition()->setTimeStep(60.0);

    loitering.Environment()->CentralBody()->GravityModel()->setModelName("EGM2008");
    loitering.Environment()->CentralBody()->GravityModel()->setNumberOfTesserals(1);
    loitering.Environment()->CentralBody()->GravityModel()->setNumberOfZonals(1);

    loitering.ElementIdentifier()->setName("loitering");
    loitering.ElementIdentifier()->setTheOrder(1);
    loitering.ElementIdentifier()->setModelName("iss.3ds");
    loitering.ElementIdentifier()->setColorName("yellow");

    return loitering;

}

void MissionsDefaults_ATV(ScenarioLoiteringType* loiteringATV)
{
}


// Launchers
void MissionsDefaults_VEGA(ScenarioLoiteringType* loiteringVEGA)
{
}

void MissionsDefaults_Soyuz(ScenarioLoiteringType* loiteringSoyuz)
{
}

void MissionsDefaults_Ariane5(ScenarioLoiteringType* loiteringAriane5)
{
}

void MissionsDefaults_IXV(ScenarioLoiteringType* loiteringIXV)
{
}


