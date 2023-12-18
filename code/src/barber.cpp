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
    while(_salon->isInService() or _salon->getNbClient() >= 0){

       // Si il n'y a aucun client, le barbier va dormir
        if(_salon->getNbClient() == 0){
           // Si le salon est fermé, c'est la fin de journée pour le barbier, il s'est occupé de tous les clients
           if(!_salon->isInService()){
              break;
           }
            _salon->goToSleep();
        }

        if(_salon->getNbClient() > 0){
           // Le barbier appelle le prochain client
           _salon->pickNextClient();

           // Le barbier attend que le client arrive sur la chaise de travail
           _salon->waitClientAtChair();

           // Le barbier coupe les cheveux du client
           _salon->beautifyClient();
        }
    }
    // Le barbier a fini sa journée
    _interface->consoleAppendTextBarber("La journée est terminée, à demain !");
}
