/*
 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA 02111-1307, USA.
 Further information about the GNU Lesser General Public License can also be found on
 the world wide web at http://www.gnu.org.
 */

/*
 ------ Copyright (C) 2008-2010 European Space Agency (space.trajectory.analysis AT gmail.com) ----
 ------------------ Author: Chris Laurel  -------------------------------------------------
 ------------------ E-mail: (claurel@gmail.com) ----------------------------
  Modified by Tiziana Sabatini May 2009
  Modified by Guillermo June 3rd 2009
  Modified by Valentino Zuccarelli June 14th 2009
  Extensvely modified by Guillermo on October 2009 to include TLEs
 */

#include "scenarioelementbox.h"
#include "sheetDelegate.h"
#include "Scenario/scenario.h"
#include "Astro-Core/stamath.h"
#include <QPushButton>
#include <QHeaderView>
#include <QMimeData>
#include <QTextStream>
#include <QtDebug>
#include "Coverage/maincoveragegui.h"

static const QString SCENARIO_MIME_TYPE("application/sta.scenario.xml");

const QString ScenarioElementBox::PARTICIPANT_MIME_TYPE("application/sta.participant.xml");
const QString ScenarioElementBox::MISSION_ARC_MIME_TYPE("application/sta.missionarc.xml");
const QString ScenarioElementBox::MANEUVER_MIME_TYPE("application/sta.maneuver.xml");
const QString ScenarioElementBox::PAYLOAD_MIME_TYPE("application/sta.payload.xml"); //Line added by Ricardo Noriega

static const int ScenarioFragmentRole     = Qt::UserRole + 1;
static const int ScenarioFragmentTypeRole = Qt::UserRole + 2;

ScenarioElementWidget::ScenarioElementWidget(QWidget* parent) :
    QTreeWidget(parent)
{
    setHeaderHidden(true);
    setRootIsDecorated(false);
    //setIconSize(QSize(32, 32));
    setIconSize(QSize(30, 30)); // Smaller icons are a bit better
    //QFont font("Helvetica", 10); setFont(font);
    setItemDelegate(new SheetDelegate(this));

    setDragDropMode(DragOnly);
    model()->setSupportedDragActions(Qt::CopyAction);
}


QStringList ScenarioElementWidget::mimeTypes() const
{
    QStringList types;
    types << SCENARIO_MIME_TYPE;
    return types;
}


QMimeData* ScenarioElementWidget::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QString mimeType;

    foreach (QTreeWidgetItem* item, items)
    {
        if (item)
        {
            QString itemMimeType = item->data(0, ScenarioFragmentTypeRole).toString();
            if (!itemMimeType.isEmpty())
            {
                if (mimeType.isEmpty())
                {
                    mimeType = itemMimeType;
                }

                // Mime types of all dragged items must match mime type of
                // first item.
                if (mimeType == itemMimeType)
                {
                    encodedData += item->data(0, ScenarioFragmentRole).toByteArray();
                }
            }
        }
    }

    mimeData->setData(mimeType, encodedData);

    return mimeData;
}


static QString fragmentText(QDomElement e)
{
    QDomDocumentFragment fragment = e.toDocumentFragment();
    QString text;
    QTextStream stream(&text, QIODevice::WriteOnly);
    e.save(stream, 1);

    return text;
}

static QByteArray REVFragment(const char* name, const char* vehicleType)//Created by Dominic to allow REV to be dragged and dropped
{
    ScenarioREV entryVehicle;

    /*** fill in defaults ***/
    entryVehicle.REVSystem()->Weights()->setTotalDryMass(100);
    entryVehicle.REVSystem()->Geometry()->setNoseRadius(1);

    entryVehicle.REVSystem()->AeroThermodynamics()->setReferenceArea(0.5*0.5*3.141592653589793238);
    entryVehicle.REVSystem()->AeroThermodynamics()->setTempCDfile("CdM.stad");
    entryVehicle.REVSystem()->AeroThermodynamics()->setCoefficientType(1);

    //qDebug()<<entryVehicle.REVSystem()->REVAeroThermodynamics()->tempCDfile()<<"  file test";

    QDomDocument doc;
    return fragmentText(CreateREVElement(&entryVehicle, doc)).toUtf8();
}

//Lines added by Ricardo Noriega. Creates a communication payload fragment representing a single payload
static QByteArray CommPayloadFragment(const char* name, const char* payloadType)
{
    ScenarioCommunicationPayloadType commPayload;


    ScenarioAntennaType * antenna=new ScenarioAntennaType;
    /*** fill in defaults ***/
    //if(commPayload.Antenna().length()!=0){
    //ScenarioAntennaType* antenna = commPayload.Antenna().at(0).data();


    //ScenarioReceiver* receiver = dynamic_cast<ScenarioReceiver*>(antenna);

    antenna->PointingDirection()->setElevation(sta::Pi()/2);
    antenna->PointingDirection()->setAzimuth(0);

    antenna->EMproperties()->setEfficiency(55);
    //antenna->EMproperties()->setFrequency(14.5);


                                   /*if (dynamic_cast<ScenarioReceiver*>(antenna) != NULL)
                                    {
                                        // It's a receiver
                                        ScenarioReceiver* receiver = dynamic_cast<ScenarioReceiver*>(antenna);
                                        // Do something with the receiver
                                        receiver->setDepointingLossRx(0);
                                        receiver->setFeederLossRx(0);
                                    }
                                    else if (dynamic_cast<ScenarioTransmitter*>(antenna) != NULL)
                                    {
                                        // It's a transmitter
                                        ScenarioTransmitter* transmitter = dynamic_cast<ScenarioTransmitter*>(antenna);
                                        // Do something with the transmitter
                                        transmitter->setDepointingLossTx(0);
                                        transmitter->setFedderLossTx(0);
                                    }*/



    commPayload.Antenna().append(QSharedPointer<ScenarioAntennaType>(antenna));
    qDebug()<<commPayload.Antenna().length()<<" length at the very beginning... why it is one?";
    //}
    QDomDocument doc;

    return fragmentText(CreateCommunicationPayloadElement(&commPayload, doc)).toUtf8();
}




// Create a scenario fragment representing a single space vehicle.
static QByteArray spaceVehicleFragment(const char* name, const char* vehicleType)
{
    ScenarioSC spacecraft;

    /*** fill in defaults ***/

    QDomDocument doc;
    return fragmentText(CreateSCElement(&spacecraft, doc)).toUtf8();
}


static QByteArray loiteringFragment(const char* name)
{
    ScenarioLoiteringType loitering;

    /*** fill in defaults ***/
    loitering.Environment()->CentralBody()->setName("EARTH");
    loitering.InitialPosition()->setCoordinateSystem("INERTIAL J2000");

    QSharedPointer<ScenarioKeplerianElementsType> initPos(new ScenarioKeplerianElementsType());
    initPos->setSemiMajorAxis(8000.0);
    loitering.InitialPosition()->setAbstract6DOFPosition(initPos);

    loitering.TimeLine()->setStartTime(QDateTime(QDate(2010, 1, 1)));
    loitering.TimeLine()->setEndTime(QDateTime(QDate(2010, 1, 2)));
    loitering.TimeLine()->setStepTime(60.0);

    loitering.PropagationPosition()->setPropagator("TWO BODY");
    loitering.PropagationPosition()->setTimeStep(60.0);

    QDomDocument doc;
    return fragmentText(CreateLoiteringElement(&loitering, doc)).toUtf8();
}


static QByteArray reentryFragment(const char* name)//Modified by Dominic to allow dragging of entry Arc
{
    ScenarioEntryArcType entry;

    // fill in defaults
    entry.Environment()->CentralBody()->setName("EARTH");
    entry.InitialPosition()->setCoordinateSystem("INERTIAL J2000");

    QSharedPointer<ScenarioSphericalCoordinatesType> initPos(new ScenarioSphericalCoordinatesType());
    initPos->setRadialDistance(6578);
    initPos->setFlightPathVelocity(6);
    initPos->setFlightPathAngle(sta::degToRad(-5.0));
    entry.InitialPosition()->setAbstract6DOFPosition(initPos);

    entry.PropagationPosition()->setTimeStep(5);
    entry.Constraints()->setMaxNormalLoad(10000.0);
    entry.Constraints()->setMaxHeatFlux(500);
    entry.Constraints()->setMaxAltitude(10000000);

    QDomDocument doc;
    return fragmentText(CreateEntryArcElement(&entry, doc)).toUtf8();
}


static QByteArray loiteringTLEFragment(const char* name)
{
    ScenarioLoiteringTLEType loiteringTLE;

    /*** fill in defaults ***/
    loiteringTLE.TimeLine()->setEndTime(QDateTime(QDate(2009, 10, 11), QTime(12, 0, 0)));
    loiteringTLE.TimeLine()->setStartTime(QDateTime(QDate(2009, 10, 12), QTime(12, 0, 0)));
    loiteringTLE.TimeLine()->setStepTime(60.0);
    loiteringTLE.setTleLine0("ISS (ZARYA)");
    loiteringTLE.setTleLine1("1 25544U 98067A   09282.91732946  .00013034  00000-0  90850-4 0  7559");
    loiteringTLE.setTleLine2("2 25544  51.6398  77.7469 0007731 180.0753 317.5762 15.74717336624025");

    QDomDocument doc;
    return fragmentText(CreateLoiteringTLEElement(&loiteringTLE, doc)).toUtf8();
}




static QByteArray externalFragment(const char* name)
{
    QByteArray data;

    data += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    data += "<External Name=\"";
    data += name;
    data += "\">";

    data +=
    "<Timeline>"
    "  <StartTime>2010-01-01T00:00:00</StartTime>"
    "  <StepTime unit=\"s\">60.0</StepTime>"
    "  <EndTime>2010-01-01T01:01:00</EndTime>"
    "</Timeline>"
    "<StateVector1>2455563, 6778, 0, 0, 0, 4.76335586717, 6.00985940331</StateVector1>"
    "<StateVector2>2455563.0006944, 6762.38865439, 285.581895497, 360.314678963, -0.520178335644, 4.75238472603, 5.99601726814</StateVector2>"
    "</External>";

    return data;
}



static QByteArray groundStationFragment(const char* name,
                                        const char* centralBody,
                                        double latitude = 0.0,
                                        double longitude = 0.0,
                                        double altitude = 0.0)
{
    ScenarioGroundStation groundStation;

    groundStation.setName(name);
    groundStation.Location()->setCentralBody(centralBody);

    QSharedPointer<ScenarioGroundPositionType> groundPosition(new ScenarioGroundPositionType());
    groundPosition->setLatitude(latitude);
    groundPosition->setLongitude(longitude);
    groundPosition->setAltitude(altitude);
    groundStation.Location()->setAbstract3DOFPosition(groundPosition);

    QDomDocument doc;
    return fragmentText(CreateGroundStationElement(&groundStation, doc)).toUtf8();
}


// Create a scenario fragment representing a ground element.
static QByteArray groundElementFragment(const char* name,
                                        const char* groundElementType,
                                        const char* centralBody,
                                        double latitude = 0.0,
                                        double longitude = 0.0,
                                        double altitude = 0.0)
{
    QByteArray encodedData;

    encodedData +=
    "<";
    encodedData += groundElementType;
    encodedData += " Name=\"";
    encodedData += name;
    encodedData += "\">"
    "<Location>"
    "<GroundPosition>"
    "<Latitude>" + QString::number(latitude) + "</Latitude>"
    "<Longitude>" + QString::number(longitude) + "</Longitude>"
    "<Altitude>" + QString::number(altitude) + "</Altitude>"
    "</GroundPosition>";
    encodedData += "<CentralBody>";
    encodedData += centralBody;
    encodedData += "</CentralBody>"
    "</Location>";

    // Clearing altitude is a required element for launch pads.
    if (QString(groundElementType) == "LaunchPad")
    {
        encodedData += "<LaunchPadClearingAltitude>10</LaunchPadClearingAltitude>";
    }

    encodedData += "</";
    encodedData += groundElementType;
    encodedData += ">";

    return encodedData;
}


// Set the mime type and data for drag and drop
static void setDragAndDropInfo(QTreeWidgetItem* item,
                               const QString& mimeType,
                               const QByteArray& mimeData)
{
    item->setData(0, ScenarioFragmentTypeRole, mimeType);
    item->setData(0, ScenarioFragmentRole, mimeData);
}


static void
addGroundStationItem(QTreeWidgetItem* parent,
                     const char* name,
                     const char* centralBody,
                     double latitude,
                     double longitude,
                     double altitude)
{
    QTreeWidgetItem* groundStationItem   = new QTreeWidgetItem(parent);
    groundStationItem->setText(0, name);
    groundStationItem->setIcon(0, QIcon(":/icons/ParticipantSTATION.png"));
    setDragAndDropInfo(groundStationItem,
                       ScenarioElementBox::PARTICIPANT_MIME_TYPE,
                       groundStationFragment(name, centralBody,
                                             latitude, longitude, altitude));
}


// New method by Guillermo to handle special satellites from ESA
static void
addESASatelliteItem(QTreeWidgetItem* parent,
                     const char* name)
{
    QTreeWidgetItem* myESASatelliteItem   = new QTreeWidgetItem(parent);
    myESASatelliteItem->setText(0, name);
    myESASatelliteItem->setIcon(0, QIcon(":/icons/ParticipantSATELLITE.png"));
    setDragAndDropInfo(myESASatelliteItem,
                       ScenarioElementBox::PARTICIPANT_MIME_TYPE,
                       spaceVehicleFragment(name, "Satellite"));
}



ScenarioElementBox::ScenarioElementBox(QWidget* parent) :
    QWidget(parent)
{

/*
// Changing the standard font of this box
QFont font("Helvetica", 10);  //QFont font("Helvetica",10,QFont::Condensed);
setFont(font);
*/


#if 0
    setupUi(this);

    int num = ScenarioElementTree->topLevelItemCount();
    ScenarioElementTree->setItemDelegate(new SheetDelegate(ScenarioElementTree));
    for(int i=0; i<num; i++)
    {
        ScenarioElementTree->setItemExpanded(ScenarioElementTree->topLevelItem(i), true);
    }
#else
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_elementTreeWidget = new ScenarioElementWidget();
    layout->addWidget(m_elementTreeWidget);

    QTreeWidgetItem* participantsItem = new QTreeWidgetItem();
    participantsItem->setText(0, tr("Participants"));
    QTreeWidgetItem* missionArcsItem = new QTreeWidgetItem();
    missionArcsItem->setText(0, tr("Mission Arcs"));
    // Next lines patch by Ricardo Noriega to include payloads into the scenario tree box
    QTreeWidgetItem* payloadsItem = new QTreeWidgetItem();
    payloadsItem->setText(0, tr("Payloads"));
    QTreeWidgetItem* maneuversItem = new QTreeWidgetItem();
    maneuversItem->setText(0, tr("Maneuvers"));

    // Now creating optional widgets to ease the work of the user:
    // ESA satellites
    QTreeWidgetItem* ESASatellitesItem = new QTreeWidgetItem();
    ESASatellitesItem->setText(0, tr("ESA Satellites"));

    // Ground stations
    QTreeWidgetItem* ESAgroundStationsItem = new QTreeWidgetItem();
    ESAgroundStationsItem->setText(0, tr("ESA Ground Stations"));
    // Next lines patch by Guillermo to include NASA stations
    QTreeWidgetItem* NASAgroundStationsItem = new QTreeWidgetItem();
    NASAgroundStationsItem->setText(0, tr("NASA Ground Stations"));


    // Adding the widgets
    m_elementTreeWidget->clear();
    m_elementTreeWidget->addTopLevelItem(participantsItem);
    m_elementTreeWidget->addTopLevelItem(missionArcsItem);
    m_elementTreeWidget->addTopLevelItem(payloadsItem);
    m_elementTreeWidget->addTopLevelItem(maneuversItem);
    m_elementTreeWidget->addTopLevelItem(ESASatellitesItem);
    m_elementTreeWidget->addTopLevelItem(ESAgroundStationsItem);
    m_elementTreeWidget->addTopLevelItem(NASAgroundStationsItem);


    /////////// Creating the widgets
    QTreeWidgetItem* satelliteItem       = new QTreeWidgetItem(participantsItem);
    satelliteItem->setText(0, tr("Satellite"));
    satelliteItem->setIcon(0, QIcon(":/icons/ParticipantSATELLITE.png"));
    setDragAndDropInfo(satelliteItem,
                       PARTICIPANT_MIME_TYPE,
                       spaceVehicleFragment("New Satellite", "Satellite"));

    QTreeWidgetItem* targetItem          = new QTreeWidgetItem(participantsItem);
    targetItem->setText(0, tr("Point"));
    targetItem->setIcon(0, QIcon(":/icons/ParticipantPOINT.png"));
    setDragAndDropInfo(targetItem,
                       PARTICIPANT_MIME_TYPE,
                       groundElementFragment("New Point", "Point", "Earth"));

    // Diabling the complete function for the time being. Patched by Guillermo
    //QTreeWidgetItem* launchPadItem       = new QTreeWidgetItem(participantsItem);
    //launchPadItem->setText(0, tr("Launch Pad"));
    //launchPadItem->setIcon(0, QIcon(":/icons/participantLaunchPad.png"));
    //setDragAndDropInfo(launchPadItem,
    //                   PARTICIPANT_MIME_TYPE,
    //                   groundElementFragment("New Launch Pad", "LaunchPad", "Earth"));

    // Diabling the complete function for the time being. Patched by Guillermo
    //QTreeWidgetItem* launcherItem        = new QTreeWidgetItem(participantsItem);
    //launcherItem->setText(0, tr("Launcher"));
    //launcherItem->setIcon(0, QIcon(":/icons/ParticipantROCKET.png"));
    //setDragAndDropInfo(launcherItem,
    //                   PARTICIPANT_MIME_TYPE,
    //                   spaceVehicleFragment("New Launcher", "Launcher"));

    QTreeWidgetItem* reentryVehicleItem  = new QTreeWidgetItem(participantsItem);
    reentryVehicleItem->setText(0, tr("Reentry vehicle"));
    reentryVehicleItem->setIcon(0, QIcon(":/icons/ParticipantENTRYVEHICLE.png"));
    setDragAndDropInfo(reentryVehicleItem,
                       PARTICIPANT_MIME_TYPE,
                       REVFragment("New Reentry Vehicle", "Reentry Vehicle"));
    //These lines are added by Ricardo to create the widget of the communication payload
    QTreeWidgetItem* communicationPayloadItem  = new QTreeWidgetItem(payloadsItem);
    communicationPayloadItem->setText(0, tr("Communication Payload"));
    communicationPayloadItem->setIcon(0, QIcon(":/icons/ParticipantENTRYVEHICLE.png"));
    setDragAndDropInfo(communicationPayloadItem,
                       PAYLOAD_MIME_TYPE,
                       CommPayloadFragment("New Communication Payload", "Comm Payload"));

    QTreeWidgetItem* groundStationItem   = new QTreeWidgetItem(participantsItem);
    groundStationItem->setText(0, tr("Station"));
    groundStationItem->setIcon(0, QIcon(":/icons/ParticipantSTATION.png"));
    setDragAndDropInfo(groundStationItem,
                       PARTICIPANT_MIME_TYPE,
                       groundElementFragment("New Ground Station", "GroundStation", "Earth"));

    // Diabling the complete function for the time being. Patched by Guillermo
    //QTreeWidgetItem* ascentItem       = new QTreeWidgetItem(missionArcsItem);
    //ascentItem->isDisabled();
    //ascentItem->setText(0, tr("Ascent"));
    //ascentItem->setIcon(0, QIcon(":/icons/mission-arcs-ascent.png"));

    QTreeWidgetItem* loiteringItem    = new QTreeWidgetItem(missionArcsItem);
    loiteringItem->setText(0, tr("Loitering"));
    loiteringItem->setIcon(0, QIcon(":/icons/mission-arcs-loitering.png"));
    setDragAndDropInfo(loiteringItem,
		       MISSION_ARC_MIME_TYPE,
		       loiteringFragment("Loitering 1"));

    QTreeWidgetItem* loiteringTLEItem    = new QTreeWidgetItem(missionArcsItem);
    loiteringTLEItem->setText(0, tr("TLE"));
    loiteringTLEItem->setIcon(0, QIcon(":/icons/TLE-icon-small.png"));
    setDragAndDropInfo(loiteringTLEItem,
		       MISSION_ARC_MIME_TYPE,
		       loiteringTLEFragment("LoiteringTLE 1"));

    QTreeWidgetItem* reentryItem      = new QTreeWidgetItem(missionArcsItem);
    reentryItem->setText(0, tr("Entry"));
    reentryItem->setIcon(0, QIcon(":/icons/mission-arcs-reentry.png"));
    setDragAndDropInfo(reentryItem,
                       MISSION_ARC_MIME_TYPE,
                       reentryFragment("ReEntry 1"));

#ifdef OLDSCENARIO
    QTreeWidgetItem* externalItem    = new QTreeWidgetItem(missionArcsItem);
    externalItem->setText(0, tr("External"));
    externalItem->setIcon(0, QIcon(":/icons/external.png"));
    setDragAndDropInfo(externalItem,
		       MISSION_ARC_MIME_TYPE,
		       externalFragment("External 1"));

    QTreeWidgetItem* lagrangianItem    = new QTreeWidgetItem(missionArcsItem);
    lagrangianItem->setText(0, tr("Lagrangian"));
    lagrangianItem->setIcon(0, QIcon(":/icons/3rdbody_icon.png"));
    setDragAndDropInfo(lagrangianItem,
                       MISSION_ARC_MIME_TYPE,
                       lagrangianFragment("Lagrangian 1"));

    // Diabling the complete function for the time being. Patched by Guillermo
    //QTreeWidgetItem* flybyItem        = new QTreeWidgetItem(missionArcsItem);
    //flybyItem->setText(0, tr("Fly-by"));
    //flybyItem->setIcon(0, QIcon(":/icons/mission-arcs-flyby.png"));
    //setDragAndDropInfo(flybyItem,
    //		       MISSION_ARC_MIME_TYPE,
    //		       flybyFragment("Fly-by 1"));



    QTreeWidgetItem* impulseItem      = new QTreeWidgetItem(maneuversItem);
    impulseItem->setText(0, tr("Single impulse"));
    impulseItem->setIcon(0, QIcon(":/icons/engine.png"));
    setDragAndDropInfo(impulseItem,
		       MISSION_ARC_MIME_TYPE,
		       impulseFragment("Single-impulse 1"));
#endif


    // Diabling the complete function for the time being. Patched by Guillermo
    //QTreeWidgetItem* rendezvousItem   = new QTreeWidgetItem(maneuversItem);
    //rendezvousItem->setText(0, tr("Rendezvous plan"));
    //rendezvousItem->setIcon(0, QIcon(":/icons/mission-arcs-rendezvous.png"));
    //setDragAndDropInfo(rendezvousItem,
    //                   MISSION_ARC_MIME_TYPE,
    //                   rendezvousFragment("Rendezvous 1"));


    // Some  ESA Satellites
    addESASatelliteItem(ESASatellitesItem, "Cryosat-2");


    // The ESTRACK ground stations (LAT, LON, ALT)
    addGroundStationItem(ESAgroundStationsItem, "Cebreros",     "Earth",  40.45268986,  -4.36754881, 794.1);
    addGroundStationItem(ESAgroundStationsItem, "Kiruna",      "Earth",  67.8571252,   20.96434169, 402.275);
    addGroundStationItem(ESAgroundStationsItem, "Kourou",      "Earth",   5.25143694, -52.8046624,   14.561);
    addGroundStationItem(ESAgroundStationsItem, "Malargue",    "Earth",  -35.775972,     -69.398250,  1525.20);
    addGroundStationItem(ESAgroundStationsItem, "Malindi",     "Earth",  -2.9955576,   40.194505,    12.314);
    addGroundStationItem(ESAgroundStationsItem, "Maspalomas",  "Earth",  27.762892,   -15.6338072,  204.9);
    addGroundStationItem(ESAgroundStationsItem, "New Norcia",  "Earth", -31.048225,   116.191500,   252.26);
    addGroundStationItem(ESAgroundStationsItem, "Perth",       "Earth", -31.802525,   115.8851556,   22.156);
    addGroundStationItem(ESAgroundStationsItem, "Redu",        "Earth",  50.0020552,    5.146231,   385.583);
    addGroundStationItem(ESAgroundStationsItem, "Santa Maria",  "Earth",  36.99694,    -25.1361027,   276.00);
    addGroundStationItem(ESAgroundStationsItem, "Santiago",     "Earth",  -33.151478,    -70.668306,   730.853);
    addGroundStationItem(ESAgroundStationsItem, "Svalbard",     "Earth",   78.229772,    15.407786,   501.3);
    //addGroundStationItem(ESAgroundStationsItem, "Villafranca", "Earth",  40.4455932,   -3.95260078, 664.425);


    // The NASA ground stations
    addGroundStationItem(NASAgroundStationsItem, "Camberra",	 "Earth", -35.4047,  148.9831, 680.0);
    addGroundStationItem(NASAgroundStationsItem, "Cape Canaveral","Earth", 28.4472,   -80.5649, -7.21);
    addGroundStationItem(NASAgroundStationsItem, "Colorado",    "Earth", 38.8060, -104.5285,   1907.51);
    addGroundStationItem(NASAgroundStationsItem, "Diego Garcia","Earth", -7.27,    72.37,  -68.38);
    addGroundStationItem(NASAgroundStationsItem, "Edwards",     "Earth", 34.9607,   -117.9116,    12.314);
    addGroundStationItem(NASAgroundStationsItem, "Goldstone",   "Earth", 35.2999,   -116.8054,  969.67);
    addGroundStationItem(NASAgroundStationsItem, "Oahu",	"Earth", 21.5721,   -158.2666,   301.32);
    addGroundStationItem(NASAgroundStationsItem, "Robledo",	"Earth", 40.4256,   -4.2541,   883.73);
    addGroundStationItem(NASAgroundStationsItem, "Vanderberg",  "Earth", 34.5827,    -120.5616,   627.54);
    addGroundStationItem(NASAgroundStationsItem, "Wallops",     "Earth", 37.9455,    -75.4611,   -18.51);
    addGroundStationItem(NASAgroundStationsItem, "White Sands", "Earth", 32.5005,    -106.6086,   1450.17);


    // Set properties for all top level items: expanded = true
    for (int i = 0; i < m_elementTreeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_elementTreeWidget->topLevelItem(i);
        m_elementTreeWidget->setItemExpanded(item, true);
        item->setExpanded(true);
    }

    // Contracting now some elements that should not be shown to the user at a first sight.
    ESASatellitesItem->setExpanded(false);
    ESAgroundStationsItem->setExpanded(false);
    NASAgroundStationsItem->setExpanded(false);


    setLayout(layout);
#endif
}


ScenarioElementBox::~ScenarioElementBox()
{
}


#ifdef OLDSCENARIO

// Obsolete code. There's no need to construct objects from inline XML;
// we can just call the appropriate scenario object constructors and set
// the object properties.

static QByteArray interplanetaryFragment(const char* name)
{
    QByteArray data;

    data += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    data += "<Interplanetary Name=\"";
    data += name;
    data += "\">";
    data +=
    "<Mode>1</Mode>";

    return data;
}

static QByteArray lagrangianFragment(const char* name)
{
    QByteArray data;
    data += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    data += "<Lagrangian Name=\"";
    data += name;
    data += "\">";

    data +=
    "<Threebodyenvironment>"
    "<FirstBody Name=\"SUN\">"
    "</FirstBody>"
    "<SecondBody Name=\"EARTH\">"
    "</SecondBody>"
    "</Threebodyenvironment>"
    "<SimulationParameters>"
    "  <Timeline>"
    "  <StartTime>2011-01-01T00:00:00</StartTime>"
    "  <StepTime unit=\"s\">60.0</StepTime>"
    "  <EndTime>2011-01-02T12:00:00</EndTime>"
    "  </Timeline>"
    "  <InitialStatePosition>"
    "  <CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "  <KeplerianElements>"
    "    <SemiMajorAxis>6772</SemiMajorAxis>"
    "    <Eccentricity>0</Eccentricity>"
    "    <Inclination>0</Inclination>"
    "    <RAAN>0</RAAN>"
    "    <ArgumentOfPeriapsis>0</ArgumentOfPeriapsis>"
    "    <TrueAnomaly>0</TrueAnomaly>"
    "  </KeplerianElements>"
        "   <CentralsystemBody Name=\"EARTH\"></CentralsystemBody>"
    "  </InitialStatePosition>"
    "  <Deltav>"
    "    <Time>0.5</Time>"
    "    <X-Deltav unit=\"km/s\">1</X-Deltav>"
    "    <Y-Deltav unit=\"km/s\">1</Y-Deltav>"
    "    <Z-Deltav unit=\"km/s\">2</Z-Deltav>"
    "  </Deltav>"
    "</SimulationParameters>"
    "<HaloOrbit>"
    "<Lpoint>L1</Lpoint>"
    "<InitialStatePosition>"
    "<CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "<StateVector>"
    "<x>0</x>"
    "<y>0</y>"
    "<z>0</z>"
    "<vx>0</vx>"
    "<vy>0</vy>"
    "<vz>0</vz>"
    "</StateVector>"
    "<CentralsystemBody Name=\"EARTH\"></CentralsystemBody>"
    "</InitialStatePosition>"
    "<Amplitudes>"
    "<xAmplitude>550000</xAmplitude>"
    "<zAmplitude>-250000</zAmplitude>"
    "</Amplitudes>"
    "<Manifolds>"
    "<Branch>"
    "<State>true</State>"
    "<Parameters>"
    "<Deviation>0</Deviation>"
    "<EndCondition>"
    "<intersection>1</intersection>"
    "<integrationTime>0</integrationTime>"
    "<numPositions>0</numPositions>"
    "</EndCondition>"
    "</Parameters>"
    "</Branch>"
    "<Branch>"
    "<State>false</State>"
    "<Parameters>"
    "<Deviation>0</Deviation>"
    "<EndCondition>"
    "<intersection>0</intersection>"
    "<integrationTime>1</integrationTime>"
    "<numPositions>30</numPositions>"
    "</EndCondition>"
    "</Parameters>"
    "</Branch>"
    "<Branch>"
    "<State>false</State>"
    "<Parameters>"
    "<Deviation>0</Deviation>"
    "<EndCondition>"
    "<intersection>1</intersection>"
    "<integrationTime>0</integrationTime>"
    "<numPositions>0</numPositions>"
    "</EndCondition>"
    "</Parameters>"
    "</Branch>"
    "<Branch>"
    "<State>false</State>"
    "<Parameters>"
    "<Deviation>0</Deviation>"
    "<EndCondition>"
    "<intersection>1</intersection>"
    "<integrationTime>0</integrationTime>"
    "<numPositions>0</numPositions>"
    "</EndCondition>"
    "</Parameters>"
    "</Branch>"
    "</Manifolds>"
    "</HaloOrbit>"
    "<ThreebodyTransfer>"
    "<StartingEpoch>2011-01-02T12:00:00</StartingEpoch>"
    "<ParkingOrbit>"
    "<InitialStatePosition>"
    "  <CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "  <KeplerianElements>"
    "    <SemiMajorAxis>6672</SemiMajorAxis>"
    "    <Eccentricity>0</Eccentricity>"
    "    <Inclination>5</Inclination>"
    "    <RAAN>0</RAAN>"
    "    <ArgumentOfPeriapsis>0</ArgumentOfPeriapsis>"
    "    <TrueAnomaly>0</TrueAnomaly>"
    "  </KeplerianElements>"
    "   <CentralsystemBody Name=\"EARTH\"></CentralsystemBody>"
    "  </InitialStatePosition>"
    "</ParkingOrbit>"
    "<HaloOrbit>"
    "<InitialStatePosition>"
    "<CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "<StateVector>"
    "<x>1.0081748644</x>"
    "<y>0</y>"
    "<z>0.0013593836</z>"
    "<vx>0</vx>"
    "<vy>0.0104429106</vy>"
    "<vz>0</vz>"
    "</StateVector>"
    "<CentralsystemBody Name=\"EARTH\"></CentralsystemBody>"
    "</InitialStatePosition>"
    "</HaloOrbit>"
    "<OptimizationParameters>"
    "<timeofFlight>0</timeofFlight>"
    "<maximumToF units=\"days\">100</maximumToF>"
    "<propellant>1</propellant>"
    "</OptimizationParameters>"
    "</ThreebodyTransfer>"
    "</Lagrangian>";

    return data;
}


static QByteArray reentryFragment(const char* name)
{
    QByteArray data;

    QString xmlTemplate = QString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<ReEntry Name=\"%1\">"
    "<Environment>"
    "<CentralBody Name=\"EARTH\"></CentralBody>"
    "</Environment>"
    "<SimulationMode>"
    " <SimulationParameters>"
    "  <Timeline>"
    "  <StartTime>2011-01-01T12:00:00</StartTime>"
    "  <StepTime unit=\"s\">60.0</StepTime>"
    "  </Timeline>"
    "  <InitialStatePosition>"
    "  <CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "  <SphericalCoordinates>"
    "    <Altitude>120</Altitude>"
    "    <Longitude>20</Longitude>"
    "    <Latitude>30</Latitude>"
    "    <InertialVelocity>12</InertialVelocity>"
    "    <InertialFlightPathAngle>-8</InertialFlightPathAngle>"
    "    <InertialHeading>90</InertialHeading>"
    "  </SphericalCoordinates>"
    "   <CentralsystemBody Name=\"EARTH\"></CentralsystemBody>"
    "  </InitialStatePosition>"
    " </SimulationParameters>"
    "</SimulationMode>"
    "<TrajectoryPropagation>"
    "  <IntegratorType>RK4</IntegratorType>"
    "  <Timestep unit=\"s\">60.0</Timestep>"
    "  <PropagatorType>TWO BODY</PropagatorType>"
    "</TrajectoryPropagation>"
    "<AttitudePropagation>"
    "  <IntegratorType>RK4</IntegratorType>"
    "  <Timestep unit=\"s\">60.0</Timestep>"
    "  <ExternCouples>NONE</ExternCouples>"
    "</AttitudePropagation>"
    "</ReEntry>"
    );

    return xmlTemplate.arg(QString(name)).toUtf8();
}


static QByteArray impulseFragment(const char* name)
{
    QByteArray data;

    QString xmlTemplate = QString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<Impulse Name=\"%1\">"
    "<Environment>"
    "<CentralBody Name=\"Earth\">"
    "<GravityModel>"
    "<ModelName>ModelName</ModelName>"
    "<NumberOfZonals>0</NumberOfZonals>"
    "<NumberOfTesserals>0</NumberOfTesserals>"
    "</GravityModel>"
    "<AtmosphereModel>AtmosphereModel</AtmosphereModel>"
    "</CentralBody>"
    "<AtmosphericDrag>0</AtmosphericDrag>"
    "<SolarPressure>0</SolarPressure>"
    "</Environment>"
    "<SimulationParameters>"
    "<Timeline>"
    "<StartTime>2011-00-00T00:00:00</StartTime>"
    "<StepTime units=\"s\">60</StepTime>"
    "</Timeline>"
    "<InitialStatePosition>"
    "<CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "<StateVector>"
    "<x>0</x>"
    "<y>0</y>"
    "<z>0</z>"
    "<vx>0</vx>"
    "<vy>0</vy>"
    "<vz>0</vz>"
    "</StateVector>"
    "</InitialStatePosition>"
    "</SimulationParameters>"
    "<Point></Point>"
    "<ManoeuvrePlan></ManoeuvrePlan>"
    "</Rendezvous>"
    );

    return xmlTemplate.arg(QString(name)).toUtf8();
}


static QByteArray rendezvousFragment(const char* name)
{
    QByteArray data;

    QString xmlTemplate = QString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<Rendezvous Name=\"%1\">"
    "<Environment>"
    "<CentralBody Name=\"Earth\">"
    "<GravityModel>"
    "<ModelName>ModelName</ModelName>"
    "<NumberOfZonals>0</NumberOfZonals>"
    "<NumberOfTesserals>0</NumberOfTesserals>"
    "</GravityModel>"
    "<AtmosphereModel>AtmosphereModel</AtmosphereModel>"
    "</CentralBody>"
    "<AtmosphericDrag>0</AtmosphericDrag>"
    "<SolarPressure>0</SolarPressure>"
    "</Environment>"
    "<SimulationParameters>"
    "<Timeline>"
    "<StartTime>2011-00-00T00:00:00</StartTime>"
    "<StepTime units=\"s\">60</StepTime>"
    "</Timeline>"
    "<InitialStatePosition>"
    "<CoordinateSystem>INERTIAL J2000</CoordinateSystem>"
    "<StateVector>"
    "<x>0</x>"
    "<y>0</y>"
    "<z>0</z>"
    "<vx>0</vx>"
    "<vy>0</vy>"
    "<vz>0</vz>"
    "</StateVector>"
    "</InitialStatePosition>"
    "</SimulationParameters>"
    "<Target></Target>"
    "<ManoeuvrePlan></ManoeuvrePlan>"
    "</Rendezvous>"
    );

    return xmlTemplate.arg(QString(name)).toUtf8();
}
#endif

