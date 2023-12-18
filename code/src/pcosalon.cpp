/**
\file pcosalon.cpp
\author Benoit Delay, Eva Ray
\date 14.12.2023

Ce fichier contient l'implémentation de la classe PcoSalon, qui permet de définir les fonctionnalités d'un salon de
coiffure concernant des clients et un barbier. Elle contient aussi des animations associées à ces fonctionnalités.
*/

#include "pcosalon.h"

#include <pcosynchro/pcothread.h>

#include <iostream>

PcoSalon::PcoSalon(GraphicSalonInterface *interface, unsigned int capacity) : _interface(interface),
                                                                              nbWaitingHaircut(0),
                                                                              isBarberSleeping(false),
                                                                              nbSeats(capacity), isOpen(true),
                                                                              nextClient(0), ticketMachine(0),
                                                                              clientWaitingAtDoor(false) {

   chairs.fill(false, capacity);
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient() {

   return nbWaitingHaircut + (clientWaitingAtDoor ? 1 : 0);
}

bool PcoSalon::accessSalon(unsigned clientId) {

   _mutex.lock();
   animationClientAccessEntrance(clientId);

   // Si la salle d'attente est pleine, le client ne peut pas accéder au salon
   if (nbWaitingHaircut >= nbSeats and (nbSeats != 0 or clientWaitingAtDoor)) {

      _interface->consoleAppendTextClient(clientId, QString("nbWaitingHaircut: %1").arg(nbWaitingHaircut));
      _interface->consoleAppendTextClient(clientId, "Je ne peux pas accéder au salon.");
      _mutex.unlock();
      return false;
   }

   _interface->consoleAppendTextClient(clientId, "Bonjour, j'aimerais une coupe!");
   int chairToSitOn;
   bool isInWaitingRoom = false;

   if (isBarberSleeping) {
      _interface->consoleAppendTextClient(clientId, "Réveillez-vous!");
      barberSleeping.notifyOne();
      isBarberSleeping = false;
      clientWaitingAtDoor = true;
   } else {
      nbWaitingHaircut++;
      // On trouve une chaise libre sur laquelle le client peut s'asseoir
      chairToSitOn = chairs.indexOf(false);
      // On signale que la chaise est maintenant occupée
      chairs[chairToSitOn] = true;
      isInWaitingRoom = true;
      animationClientSitOnChair(clientId, chairToSitOn);
   }

   unsigned int ticket = ++ticketMachine;
   // Le client attend son tour en utilisant un système de ticket. Tant qu'il n'a pas le non numéro de ticket,
   // il continue à attendre.
   while (ticket > nextClient) {
      canGoForHaircut.wait(&_mutex);
   }

   // Si le client était assis sur une chaise de la salle d'attente, on la libère
   if (isInWaitingRoom) {
      chairs[chairToSitOn] = false;
   }
   _mutex.unlock();
   return true;
}


void PcoSalon::goForHairCut(unsigned clientId) {
   _mutex.lock();
   _interface->consoleAppendTextClient(clientId, "Je vais sur la chaise pour me faire couper les cheveux.");
   animationClientSitOnWorkChair(clientId);
   // Si le client n'est pas le premier après le réveil du barbier
   if (!clientWaitingAtDoor) {
      nbWaitingHaircut--;
   }
   waitClient.notifyOne();
   doneBeautified.wait(&_mutex);
   _interface->consoleAppendTextClient(clientId, "J'ai une nouvelle coupe, stylé!");
   _mutex.unlock();
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
   _mutex.lock();
   _interface->consoleAppendTextClient(clientId, "J'attends que mes cheveux repoussent.");
   animationClientWaitForHairToGrow(clientId);
   _mutex.unlock();
}


void PcoSalon::walkAround(unsigned clientId) {
   _mutex.lock();
   _interface->consoleAppendTextClient(clientId, "Je vais faire un tour!");
   animationClientWalkAround(clientId);
   _mutex.unlock();
}


void PcoSalon::goHome(unsigned clientId) {
   _mutex.lock();
   animationClientGoHome(clientId);
   _mutex.unlock();
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep() {
   _mutex.lock();
   _interface->consoleAppendTextBarber("Je vais faire une sieste!");
   isBarberSleeping = true;
   animationBarberGoToSleep();
   barberSleeping.wait(&_mutex);
   animationWakeUpBarber();
   _mutex.unlock();
}


void PcoSalon::pickNextClient() {
   _mutex.lock();
   nextClient++;
   _interface->consoleAppendTextBarber("Au prochain!");
   canGoForHaircut.notifyAll();
   _mutex.unlock();
}


void PcoSalon::waitClientAtChair() {
   _mutex.lock();
   _interface->consoleAppendTextBarber("J'attends le prochain client.");
   waitClient.wait(&_mutex);
   _mutex.unlock();
}


void PcoSalon::beautifyClient() {
   _mutex.lock();
   _interface->consoleAppendTextBarber("Je commence à faire la coupe.");
   animationBarberCuttingHair();
   _interface->consoleAppendTextBarber("La coupe est terminée!");
   doneBeautified.notifyOne();
   if(clientWaitingAtDoor){
      // On s'est occupé du premier client qui a réveillé le barbier
      clientWaitingAtDoor = false;
   }
   _mutex.unlock();
}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService() {

   return isOpen;
}


void PcoSalon::endService() {

   _mutex.lock();
   _interface->consoleAppendTextBarber("La salon va fermer, plus de nouveaux clients!");
   isOpen = false;
   // Si le barbier est en train de dormir, on le réveille pour qu'il vienne fermer le salon
   if (isBarberSleeping) {
      barberSleeping.notifyOne();
   }
   _mutex.unlock();
}

/********************************************
 *   Méthodes privées pour les animations   *
 *******************************************/
void PcoSalon::animationClientAccessEntrance(unsigned clientId) {
   _mutex.unlock();
   _interface->clientAccessEntrance(clientId);
   _mutex.lock();
}

void PcoSalon::animationClientSitOnChair(unsigned clientId, unsigned clientSitNb) {
   _mutex.unlock();
   _interface->clientSitOnChair(clientId, clientSitNb);
   _mutex.lock();
}

void PcoSalon::animationClientSitOnWorkChair(unsigned clientId) {
   _mutex.unlock();
   _interface->clientSitOnWorkChair(clientId);
   _mutex.lock();
}

void PcoSalon::animationClientWaitForHairToGrow(unsigned clientId) {
   _mutex.unlock();
   _interface->clientWaitHairToGrow(clientId, true);
   _mutex.lock();
}

void PcoSalon::animationClientWalkAround(unsigned clientId) {
   _mutex.unlock();
   _interface->clientWalkAround(clientId);
   _mutex.lock();
}

void PcoSalon::animationBarberGoToSleep() {
   _mutex.unlock();
   _interface->barberGoToSleep();
   _mutex.lock();
}

void PcoSalon::animationWakeUpBarber() {
   _mutex.unlock();
   _interface->clientWakeUpBarber();
   _mutex.lock();
}

void PcoSalon::animationBarberCuttingHair() {
   _mutex.unlock();
   _interface->barberCuttingHair();
   _mutex.lock();
}

void PcoSalon::animationClientGoHome(unsigned clientId) {
   _mutex.unlock();
   _interface->clientWaitHairToGrow(clientId, false);
   _mutex.lock();
}
