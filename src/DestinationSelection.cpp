#include "DestinationSelection.h"


DestinationSelection::DestinationSelection(){}

int DestinationSelection::update();

void DestinationSelection::display();

char* DestinationSelection::get_destination() {
    return destination;
};

int DestinationSelection::get_destination_floor() {
    return destination_floor;
};