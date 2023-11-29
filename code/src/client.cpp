/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */
// Modifications à faire dans le fichier

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
        while(!_salon->accessSalon(_clientId)){
            _salon->walkAround(_clientId);
        }

        _salon->goForHairCut(_clientId);

        _salon->waitingForHairToGrow(_clientId);

    }
    // Tschuss mon pote
    _salon->goHome(_clientId);
    _interface->consoleAppendTextClient(_clientId, "Le salon est fermé... Zut !");
}
