/**
\file client.cpp
\author Benoit Delay, Eva Ray
\date 14.12.2023

Ce fichier contient l'implémentation de la classe Client, qui permet de
défnir le comportement d'un client d'un salon de coiffure.
*/

#include "client.h"
#include <unistd.h>

#include <iostream>

int Client::_nextId = 0;

Client::Client(GraphicSalonInterface *interface, std::shared_ptr<SalonClientInterface> salon)

    : _interface(interface), _salon(salon),  _clientId(_nextId++)
{
    _interface->consoleAppendTextClient(_clientId, "Salut, prêt pour une coupe !");
}

void Client::run()
{
    while(_salon->isInService())
    {
       // Si le client ne peut pas acéder au salon il va faire un tour
        if(!_salon->accessSalon(_clientId)){
            _salon->walkAround(_clientId);
        } else {

           // Le client va se faire couper les cheveux
           _salon->goForHairCut(_clientId);

           // Le client attend que ses cheveux repoussent
           _salon->waitingForHairToGrow(_clientId);
        }

    }
    // Le client rentre chez lui
    _salon->goHome(_clientId);
   _interface->consoleAppendTextClient(_clientId, "Je rentre chez moi pour aujourd'hui.");
}
