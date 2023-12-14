/**
\file barber.cpp
\author Benoit Delay, Eva Ray
\date 14.12.2023

Ce fichier contient l'implémentation de la classe Barber, qui permet de
défnir le comportement d'un barbier dans le salon de coiffure.
*/

#include "barber.h"
#include <unistd.h>

#include <iostream>

Barber::Barber(GraphicSalonInterface *interface, std::shared_ptr<SalonBarberInterface> salon)
    : _interface(interface), _salon(salon)
{
    _interface->consoleAppendTextBarber("Salut, prêt à travailler !");
}

void Barber::run()
{
    while(_salon->isInService()){

        if(_salon->getNbClient() == 0){
            _salon->goToSleep();
        }

        _salon->pickNextClient();

        _salon->waitClientAtChair();

        _salon->beautifyClient();
    }
    _salon->goToSleep();
    _interface->consoleAppendTextBarber("La journée est terminée, à demain !");
}
