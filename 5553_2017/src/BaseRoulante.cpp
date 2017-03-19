/*
 * BaseRoulante.cppS
 *
 *  Created on: 27 d�c. 2016
 *      Author: REBAUDET Thomas
 */

#include "WPILib.h"
#include <RobotDrive.h>
#include <BaseRoulante.h>
#include <DoubleSolenoid.h>
#include <constantes.h>



BaseRoulante::BaseRoulante():
mecaFrontLeft(0,0,1,0),mecaBackLeft(1,2,3,0),mecaFrontRight(3,4,5,0),mecaBackRight(2,6,7,0), verins_BASE(0,1)
{
		// arr�t des moteurs
		mecaFrontLeft.Set(0.0);
		mecaFrontRight.Set(0.0);
		mecaBackRight.Set(0.0);
		mecaBackLeft.Set(0.0);
		// configuration mode TANK

		verins_BASE.Set(frc::DoubleSolenoid::kReverse);

		robotMode = MODE_TANK;
		mode_auto = MODE_ALIGN;
		// configuration du robotDrive
		//R2D2 = new RobotDrive(mecaFrontLeft,mecaBackLeft,mecaFrontRight,mecaBackRight);
		approach_speed = 0.3;
		align_dist = 500; // en mm
		align_marge = 20; // en mm
		rot_marge = 10; // en mm
		rot_speed = 0.3; // entre -1 et 1
}


void BaseRoulante::SetVitesseMax(double max)
{
	mecaFrontLeft.SetVitesseMax(max);
	mecaFrontRight.SetVitesseMax(max);
	mecaBackLeft.SetVitesseMax(max);
	mecaBackRight.SetVitesseMax(max);
}

void BaseRoulante::reset()
{
	mecaFrontLeft.Reset();
	mecaFrontRight.Reset();
	mecaBackLeft.Reset();
	mecaFrontRight.Reset();
}

void BaseRoulante::parcourirDistance(double distanceGauche, double distanceDroite)
{
	consigneG=distanceGauche;
	consigneD=distanceDroite;
	std::cout<<"consigne :"<<consigneG<<std::endl;
	for(int i=0;i<Nintegration;i++)
	{
		erreursD[i]=erreursG[i]=0;
	}
	PerreurG=distanceGauche;
	PerreurD=distanceDroite;
	reset();
}

double BaseRoulante::effectuerConsigne(double P, double I, double D)
{
	double erreurD = 0,  erreurG = 0; //erreurs actuelles
	double sommeErreursG=0, differenceErreursG=0;
	double sommeErreursD=0, differenceErreursD=0;
	powerLeft=powerRight=0;

	double moyenneGauche = (mecaFrontLeft.GetDistance() + mecaBackLeft.GetDistance())/2.0f;
	double moyenneDroite = (mecaFrontRight.GetDistance() + mecaBackRight.GetDistance())/2.0f;

	SmartDashboard::PutNumber("Moyenne Gauche", moyenneGauche);
	SmartDashboard::PutNumber("Moyenne Droite", moyenneDroite);
	std::cout<<"moyenne :"<<moyenneGauche<<std::endl;

	erreurG=consigneG - moyenneGauche;
	differenceErreursG = erreurG-PerreurG;
	PerreurG=erreurG;
	erreursG[indiceIntegration]=erreurG;

	erreurD=consigneD - moyenneDroite;
	differenceErreursD = erreurD-PerreurD;
	PerreurD=erreurD;
	erreursD[indiceIntegration]=erreurD;

	indiceIntegration++;
	if(indiceIntegration>=Nintegration)
		indiceIntegration=0;

	for(int i=0;i<Nintegration;i++)
	{
		sommeErreursD+=erreursD[i];
		sommeErreursG+=erreursG[i];
	}

	powerLeft=(float)(erreurG*P /*+ D*differenceErreursG + I*sommeErreursG*/);
	powerRight=-powerLeft;

	std::cout<<"powerLeft :"<<powerLeft<<std::endl;

	mecaFrontLeft.Set(powerLeft);
	mecaFrontRight.Set(powerRight);
	mecaBackRight.Set(powerRight);
	mecaBackLeft.Set(powerLeft);

	return std::abs(erreurG);
}



void BaseRoulante::setRobotMode(int mode){
	if(mode == MODE_TANK){
		// rentrer les verins

		verins_BASE.Set(frc::DoubleSolenoid::kForward);
	}
	if(mode == MODE_MECA){
		// pousser les verins
		verins_BASE.Set(frc::DoubleSolenoid::kReverse);

	}
	// store robot mode
	robotMode = mode;
}

int BaseRoulante::getRobotMode(){
	return(robotMode);
}


void BaseRoulante::mvtJoystick(Joystick *joystick, ADXRS450_Gyro* gyro)
{
	if(robotMode == MODE_TANK){
		//R2D2->ArcadeDrive(	-joystick->GetZ(),-joystick->GetY(),true);
		float x= -((float)joystick->GetX());
		float y= -((float)joystick->GetY());
		float z= -((float)joystick->GetZ());

		if (x>=-0.2 && x<=0.2)
			x=0;

		if (y>=-0.2 && y<=0.2)
					y=0;

		if (z>=-0.3 && z<=0.3)
					z=0;

		mecaFrontRight.Set(y +zCoeff *z);
		mecaBackRight.Set(y+zCoeff *z);
		mecaFrontLeft.Set(-y+ zCoeff *z);
		mecaBackLeft.Set(-y+ zCoeff *z);

		//R2D2->ArcadeDrive(joystick);
		//R2D2->ArcadeDrive(joystick,frc::Joystick::AxisType::kZAxis,joystick,frc::Joystick::AxisType::kYAxis);
	}

	if(robotMode == MODE_MECA){

		float x= -((float)joystick->GetX());
		float y= -((float)joystick->GetY());
		float z= -((float)joystick->GetZ());

		if(x>=-0.2 && x<=0.2)
			x=0;
		if(y>=-0.2 && y<=0.2)
			y=0;
		if(z>=-0.3 && z<=0.3)
			z=0;

		mecaFrontRight.Set(+y+ -x+z);
		mecaBackRight.Set(y+x+z);
		mecaFrontLeft.Set(-y -x +z);
		mecaBackLeft.Set(-y+x+z);





		std::cout<<"x: "<<x<<std::endl;
		std::cout<<"y: "<<y<<std::endl;
		std::cout<<"z: "<<z<<std::endl;

		//R2D2->MecanumDrive_Cartesian(x,y,z,angle);
	}
}

void BaseRoulante::resetModeAuto(){
	mode_auto=MODE_APPROACH;
}



BaseRoulante::~BaseRoulante() {
	// TODO Auto-generated destructor stub

}

