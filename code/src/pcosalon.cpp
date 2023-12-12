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
#include <algorithm>

PcoSalon::PcoSalon(GraphicSalonInterface *interface, unsigned int capacity)
    : _interface(interface), nbWaitingAccess(0), nbWaitingHaircut(0), barberOccupied(false), isBarberSleeping(false)
{
    chairs.fill(false, NB_SEATS);
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient()
{
    return nbWaitingHaircut;
}

bool PcoSalon::accessSalon(unsigned clientId)
{
    _mutex.lock();

    // Si la salle d'attente est pleine
    if(nbWaitingHaircut >= NB_SEATS){

        _mutex.unlock();
        return false;
    }

    _mutex.unlock();
    return true;
    
}


void PcoSalon::goForHairCut(unsigned clientId)
{
    _mutex.lock();

    nbWaitingHaircut++;

    animationClientAccessEntrance(clientId);
    // gérer la répartition sur les chaises
    int i = chairs.indexOf(false);
    chairs[i] = true;
    animationClientSitOnChair(clientId, i);

    if(isBarberSleeping){

        barberSleeping.notifyOne();
        isBarberSleeping = false;
    } 

    canGoForHaircut.wait(&_mutex);
    nbWaitingHaircut--;

    if(nbWaitingAccess > 0){
        // on libère une place dans la sale d'attente
        chairs[i] = false;
        canAccessSalon.notifyOne();
    }

    animationClientSitOnWorkChair(clientId);
    waitClient.notifyOne();
    doneBeautified.wait(&_mutex);
    barberOccupied = false;
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
    barberSleeping.wait(&_mutex);
    animationWakeUpBarber();
    _mutex.unlock();
}


void PcoSalon::pickNextClient()
{
    _mutex.lock();
    if(nbWaitingHaircut > 0){
       canGoForHaircut.notifyOne();
    }
    _mutex.unlock();
}


void PcoSalon::waitClientAtChair()
{
    _mutex.lock();
    waitClient.wait(&_mutex);
    _mutex.unlock();
}


void PcoSalon::beautifyClient()
{
    _mutex.lock();
    barberOccupied = true;
    animationBarberCuttingHair();
    doneBeautified.notifyOne();
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
