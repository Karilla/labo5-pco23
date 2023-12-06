/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */
// Modifications à faire dans le fichier

#include "pcosalon.h"

#include <pcosynchro/pcothread.h>

#include <iostream>

PcoSalon::PcoSalon(GraphicSalonInterface *interface, unsigned int capacity)
    : _interface(interface)
{

}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient()
{
    return 1;
}

bool PcoSalon::accessSalon(unsigned clientId)
{
    _mutex.lock();
    if(nbWaitingHaircut >= NB_SEATS){

        animationClientAccessEntrance(clientId);
        // gérer la répartition sur les chaises
        animationClientSitOnChair(clientId,1);
        _mutex.unlock();
        return true;
    }

    _mutex.unlock();
    return false;
}


void PcoSalon::goForHairCut(unsigned clientId)
{
    _mutex.lock();
    if(barberOccupied){

        nbWaitingHaircut++;
        canGoForHaircut.wait(&_mutex);
        nbWaitingHaircut--;

    }

    if(nbWaitingAccess > 0){
        canAccessSalon.notifyOne();
    }
    animationClientSitOnWorkChair(clientId);
    _mutex.unlock();
}

void PcoSalon::waitingForHairToGrow(unsigned clientId)
{
    _mutex.lock();
    animationClientWaitForHairToGrow(clientId);
    _mutex.unlock();
}


void PcoSalon::walkAround(unsigned clientId)
{
    _mutex.lock();
    nbWaitingAccess++;
    animationClientWalkAround(clientId);
    canAccessSalon.wait(&_mutex);
    nbWaitingAccess--;
    _mutex.unlock();
}


void PcoSalon::goHome(unsigned clientId){

    _mutex.lock();
    if(nbWaitingHaircut > 0){
       canGoForHaircut.notifyOne();
    }
    animationClientGoHome(clientId);
    _mutex.unlock();
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep()
{
    _mutex.lock();
    isBarberSleeping = true;
    animationBarberGoToSleep();
    _mutex.unlock();
}


void PcoSalon::pickNextClient()
{
    _mutex.lock();
    canGoForHaircut.notifyOne();
    _mutex.unlock();
}


void PcoSalon::waitClientAtChair()
{

}


void PcoSalon::beautifyClient()
{
    _mutex.lock();
    animationBarberCuttingHair();
    _mutex.unlock();
}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService()
{
    return true;
    // TODO
}


void PcoSalon::endService()
{
    // TODO
}

/********************************************
 *   Méthodes privées pour les animations   *
 *******************************************/
void PcoSalon::animationClientAccessEntrance(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientAccessEntrance(clientId);
    _mutex.lock();
}

void PcoSalon::animationClientSitOnChair(unsigned clientId, unsigned clientSitNb)
{
    _mutex.unlock();
    _interface->clientSitOnChair(clientId, clientSitNb);
    _mutex.lock();
}

void PcoSalon::animationClientSitOnWorkChair(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientSitOnWorkChair(clientId);
    _mutex.lock();
}

void PcoSalon::animationClientWaitForHairToGrow(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientWaitHairToGrow(clientId, true);
    _mutex.lock();
}

void PcoSalon::animationClientWalkAround(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientWalkAround(clientId);
    _mutex.lock();
}

void PcoSalon::animationBarberGoToSleep()
{
    _mutex.unlock();
    _interface->barberGoToSleep();
    _mutex.lock();
}

void PcoSalon::animationWakeUpBarber()
{
    _mutex.unlock();
    _interface->clientWakeUpBarber();
    _mutex.lock();
}

void PcoSalon::animationBarberCuttingHair()
{
    _mutex.unlock();
    _interface->barberCuttingHair();
    _mutex.lock();
}

void PcoSalon::animationClientGoHome(unsigned clientId){
    _mutex.unlock();
    _interface->clientWaitHairToGrow(clientId, false);
    _mutex.lock();
}
