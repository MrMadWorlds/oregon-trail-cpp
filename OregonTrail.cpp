/**
 * Reference code:
 * https://github.com/LiquidFox1776/oregon-trail-1978-basic/commit/0577d0b8f43406a788be625a8ece0e7fb867d089
 * 
 * A translation of Oregon Trail from 1978
 * From BASIC to C++
 *
 * Translation by MadWorlds
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <limits>

typedef struct choiceEnd {
    bool dead;
    short choice;
} ChoiceEnd;

enum class DeathType {
    NO_FOOD,
    CANT_PAY_DOC,
    NO_MEDS,
    TOO_LATE,
    INJURIES,
};

const std::string SHOOTING_WORD_VARIATIONS[] = {
    "bang",
    "blam",
    "pow",
    "wham"
};

const std::string DATES[] = { //len - 18
    "APRIL 12 ",
    "APRIL 26 ",
    "MAY 10 ",
    "MAY 24 ",
    "JUNE 7 ",
    "JUNE 21 ",
    "JULY 5 ",
    "JULY 19 ",
    "AUGUST 2 ",
    "AUGUST 16 ",
    "AUGUST 31 ",
    "SEPTEMBER 13 ",
    "SEPTEMBER 27 ",
    "OCTOBER 11 ",
    "OCTOBER 25 ",
    "NOVEMBER 8 ",
    "NOVEMBER 22 ",
    "DECEMBER 6 ",
    "DECEMBER 20 ",
};

const std::string DAYS_OF_WEEK[] = {
    "MONDAY ",
    "TUESDAY ",
    "WEDNESDAY ",
    "THURSDAY ",
    "FRIDAY ",
    "SATURDAY ",
    "SUNDAY "
};

bool flags[9] = { false, false, false, false, false, false, false, false, false }; //(fort option) X1, (injury) K8, (illness) S4, (south pass) F1, (blue mntn) F2, (clearing south path in setting mileage) M9, (blizzard) L1, (no clothes in cold) C1, (game won) custom

unsigned short totalMileage = 0; //M
short numOfOxen = 0, eatingPref = 0; //A, E

/**
 * Returns whether or not a number is between a minimum and maximum inclusive
 * 
 * @param num The number being checked
 * @param min The minimum number
 * @param max The maximum number
 * @return Whether the number is between min and max, inclusive
 */
bool inRange(int num, int min, int max)
{
    return (num >= min) && (num <= max);
}

/**
 * For primitives only!!!
 */
unsigned short getInput()
{
    unsigned short input;

    for (;;)
    {
        std::cin >> input;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "NOT A VALID NUMBER" << std::endl;
        }
        else
            break;
    }

    return input;
}

/**
 * Generates a random float between 0-1
 */
float smallRandom()
{
    return (float)rand() / RAND_MAX;
}

/**
 * Handles the initial purchasing of supplies
 */
bool getInitialStats(short *supplies, const std::string *supplyNames, short *cashLeft)
{
    std::cout << "\n\nHOW MUCH DO YOU WANT TO SPEND ON YOUR OXEN TEAM" << std::endl;

    do {
        numOfOxen = getInput();

        if (numOfOxen < 200)
            std::cout << "NOT ENOUGH" << std::endl;
        else if (numOfOxen > 300)
            std::cout << "TOO MUCH" << std::endl;
    } while (!inRange(numOfOxen, 200, 300));

    for (int i = 0; i < 4; ++i)
    {
        std::cout << "HOW MUCH DO YOU WANT TO SPEND ON " << supplyNames[i] << std::endl;
        supplies[i] = getInput();

        do {
            if (supplies[i] < 0)
                std::cout << "IMPOSSIBLE" << std::endl;
        } while (supplies[i] < 0);
    }

    *cashLeft = 700 - numOfOxen;
    for (int i = 0; i < 4; ++i)
        *cashLeft -= supplies[i];

    return (*cashLeft >= 0);
}

/**
 * Prints the death message for the specific death that occured.
 * 
 * @param reason A DeathType enum describing the cause of death
 */
void gameOver(DeathType reason)
{
    std::cout << "\n";

    if (reason == DeathType::NO_FOOD)
        std::cout << "YOU RAN OUT OF FOOD AND STARVED TO DEATH" << std::endl;
    else if (reason == DeathType::CANT_PAY_DOC)
        std::cout << "YOU CAN'T AFFORD A DOCTOR\n";
    else if (reason == DeathType::NO_MEDS)
        std::cout << "YOU RAN OUT OF MEDICAL SUPPLIES\n";
    else if (reason == DeathType::TOO_LATE)
    {
        std::cout << "YOU HAVE BEEN ON THE TRAIL TOO LONG ------\n"
            << "YOUR FAMILY DIES IN THE FIRST BLIZZARD OF WINTER" << std::endl;
    }

    if (reason == DeathType::NO_MEDS || reason == DeathType::CANT_PAY_DOC || reason == DeathType::INJURIES)
    {
        std::cout << "YOU DIED OF ";

        if (flags[1])
            std::cout << "INJURIES" << std::endl;
        else
            std::cout << "PNEUMONIA" << std::endl;
    }
}

/**
 * Handles shooting the gun
 */
float shooting(unsigned short shootingDiff)
{
    float reactionTime = 0.0f;
    clock_t t;
    short shootingWordInd = rand() % 4;
    std::string input;

    std::cout << "TYPE " << SHOOTING_WORD_VARIATIONS[shootingWordInd] << std::endl;
    t = clock();

    std::cin >> input;
    t = clock() - t;

    reactionTime = ((float)t / CLOCKS_PER_SEC) - (shootingDiff - 1.0f);
    if (reactionTime < 0.0f)
        reactionTime = 0.0f;

    if (input != SHOOTING_WORD_VARIATIONS[shootingWordInd])
        reactionTime = 9.0f;

    return reactionTime;
}

/**
 * Processes bandits attacking you
 */
void banditDefenseEval(short *supplies, unsigned short shootingDiff)
{
    float reactionTime = shooting(shootingDiff);
    supplies[1] = supplies[1] - reactionTime * 40 - 80; //Lower bullets

    if (reactionTime <= 1)
        std::cout << "NICE SHOOTING---YOU DROVE THEM OFF" << std::endl;
    else if (reactionTime <= 4)
        std::cout << "KINDA SLOW WITH YOUR COLT .45" << std::endl;
    else
    {
        std::cout << "LOUSY SHOT---YOU GOT KNIFED" << std::endl;
        flags[1] = 1; //Injury flag
        std::cout << "YOU HAVE TO SEE OL' DOC BLANCHARD" << std::endl;
    }
}

/**
 * Processes someone getting sick
 */
bool illnessDisplay(short *supplies)
{
    float randNum = 100.0 * smallRandom();

    if (randNum < 10 + 35 * (eatingPref - 1))
    {
        std::cout << "MILD ILLNESS---MEDICINE USED" << std::endl;
        totalMileage -= 5;
        supplies[3] = supplies[3] - 2;
    }
    else if (randNum < 100 - (400 / 4 ^ (eatingPref - 1)))
    {
        std::cout << "BAD ILLNESS---MEDICINE USED" << std::endl;
        totalMileage -= 5;
        supplies[3] = supplies[3] - 5;
    }
    else
    {
        std::cout << "YOU MUST STOP FOR MEDICAL ATTENTION" << std::endl;
        supplies[3] = supplies[3] - 10;
        flags[2] = 1;
    }

    if (supplies[3] < 0)
    {
        gameOver(DeathType::NO_MEDS);
        return true;
    }

    if ((flags[6] == 1) && (totalMileage >= 2040))
        flags[8] = 1;

    return false;
}

/**
 * Processes a blizzard happening in the pass that causes set backs
 */
bool blizzardPass(short *supplies)
{
    bool result = false;

    std::cout << "BLIZZARD IN MOUNTAIN PASS--TIME AND SUPPLIES LOST" << std::endl;
    flags[6] = 1;
    supplies[0] = supplies[0] - 25;
    supplies[1] = supplies[1] - 300;
    supplies[3] = supplies[3] - 10;
    totalMileage -= 30 - 40 * smallRandom();

    if (supplies[2] < 18 + 2 * smallRandom())
        result = illnessDisplay(supplies);

    if (totalMileage <= 950)
        flags[5] = 1;

    return result;
}

/**
 * Mountain related events
 */
bool mountainProcessing(short *supplies)
{
    bool result = false;

    if (totalMileage > 950)
    {
        if ((smallRandom() * 10) <= (9 - ((totalMileage / 100 - 15) ^ 2 + 72) / ((totalMileage / 100 - 15) ^ 2 + 12)))
        {
            std::cout << "RUGGED MOUNTAINS" << std::endl;

            if (smallRandom() <= 0.1f)
            {
                std::cout << "YOU GOT LOST---LOSE VALUABLE TIME TRYING TO FIND TRAIL!" << std::endl;
                totalMileage -= 60;
            }
            else if (smallRandom() > 0.11f)
            {
                std::cout << "THE GOING GETS SLOW" << std::endl;
                totalMileage -= 45 - 50 * smallRandom();

            }
            else
            {
                std::cout << "WAGON DAMAGED!-LOSE TIME AND SUPPLIES" << std::endl;
                supplies[3] = supplies[3] - 5;
                supplies[1] = supplies[1] - 200;
                totalMileage -= 20 - 30 * smallRandom();
            }

        }

        if (!flags[3]) //first time going through south pass
        {
            flags[3] = 1;

            if (smallRandom() < 0.8f)
                result = blizzardPass(supplies);
            else
            {
                std::cout << "YOU MADE IT SAFELY THROUGH SOUTH PASS--NO SNOW" << std::endl;

                if (totalMileage < 1700 || flags[4] == 1)
                {
                    if (totalMileage <= 950)
                        flags[5] = 1;
                }
                else
                {
                    flags[4] = 1;

                    if (smallRandom() < 0.7f)
                        result = blizzardPass(supplies);
                    else if (totalMileage <= 950)
                        flags[5] = 1;
                }
            }

            //if (totalMileage <= 950)
            //    flags[5] = 1;
        }
    }

    return result;
}

/**
 * Chooses a random event to happen to the player
 * 
 */
bool selectionOfEvents(short *supplies, unsigned short shootingDiff, short *cashLeft)
{
    unsigned short eventsCounter = 0;
    float randomNum = smallRandom() * 100; //R1
    bool result = false;

    if (randomNum <= 6)
    {
        std::cout << "WAGON BREAKS DOWN--LOSE TIME AND SUPPLIES FIXING IT" << std::endl;
        totalMileage -= 15 - 5 * smallRandom();
        supplies[3] = supplies[3] - 8;
    }
    else if (randomNum <= 11)
    {
        std::cout << "OX INJURES LEG--SLOWS YOU DOWN REST OF TRIP" << std::endl;
        totalMileage -= 25;
        numOfOxen -= 20;
    }
    else if (randomNum <= 13)
    {
        std::cout << "BAD LUCK--YOUR DAUGHTER BROKE HER ARM" << std::endl;
        std::cout << "YOU HAD TO STOP AND USE SUPPLIES TO MAKE A SLING" << std::endl;
        
        totalMileage -= 5 - 4 * smallRandom();
        supplies[3] = supplies[3] - 2 - 3 * smallRandom();
    }
    else if (randomNum <= 15)
    {
        std::cout << "OX WANDERS OFF--SPEND TIME LOOKING FOR IT" << std::endl;
        totalMileage -= 17;
    }
    else if (randomNum <= 17)
    {
        std::cout << "YOUR SON GETS LOST---SPEND HALF THE DAY LOOKING FOR HIM" << std::endl;
        totalMileage -= 10;
    }
    else if (randomNum <= 22)
    {
        std::cout << "UNSAFE WATER--LOSE TIME LOOKING FOR CLEAN SPRING" << std::endl;
        totalMileage -= 10 * smallRandom() - 2;
    }
    else if (randomNum <= 32)
    {
        if (totalMileage > 950)
        {
            std::cout << "COLD WEATHER---BRRRRRRR!---YOU ";

            if (supplies[2] <= 22 + 4 * smallRandom())
            {
                std::cout << "DON'T ";
                flags[7] = 1; //no clothes in winter flag
            }
                
            std::cout << "HAVE ENOUGH CLOTHING TO KEEP YOU WARM" << std::endl;

            if (flags[7])
                result = illnessDisplay(supplies);
        }
        else
        {
            std::cout << "HEAVY RAINS---TIME AND SUPPLIES LOST" << std::endl;
            supplies[0] = supplies[0] - 10;
            supplies[1] = supplies[1] - 500;
            supplies[3] = supplies[3] - 15;
            totalMileage -= 10 * smallRandom() - 5;
        }
    }
    else if (randomNum <= 35)
    {
        float reactionTime = 0.0f;

        std::cout << "BANDITS ATTACK" << std::endl;
        reactionTime = shooting(shootingDiff);
        supplies[1] = supplies[1] - 20 * reactionTime;

        if (supplies[1] < 0)
        {
            std::cout << "YOU RAN OUT OF BULLETS---THEY GET LOTS OF CASH" << std::endl;
            *cashLeft /= 3;
        }
        else
        {
            if (reactionTime <= 1)
            {
                std::cout << "QUICKEST DRAW OUTSIDE OF DODGE CITY!!!" << std::endl;
                std::cout << "YOU GOT 'EM!" << std::endl;
            }
            else
            {
                std::cout << "YOU GOT SHOT IN THE LEG AND THEY TOOK ONE OF YOUR OXEN" << std::endl;
                flags[1] = 1; //injury
                
                std::cout << "BETTER HAVE A DOC LOOK AT YOUR WOUND" << std::endl;
                supplies[3] = supplies[3] - 5;
                numOfOxen -= 20;
            }
        }
    }
    else if (randomNum <= 37)
    {
        std::cout << "THERE WAS A FIRE IN YOUR WAGON--FOOD AND SUPPLIES DAMAGE!" << std::endl;
        supplies[0] = supplies[0] - 40;
        supplies[1] = supplies[1] - 400;
        supplies[3] = supplies[3] - smallRandom() * 8 - 3;
        totalMileage -= 15;
    }
    else if (randomNum <= 42)
    {
        std::cout << "LOSE YOUR WAY IN HEAVY FOG---TIME IS LOST" << std::endl;
        totalMileage -= 10 - 5 * smallRandom();
    }
    else if (randomNum <= 44)
    {
        std::cout << "YOU KILLED A POISONOUS SNAKE AFTER IT BIT YOU" << std::endl;
        supplies[1] = supplies[1] - 10;
        supplies[3] = supplies[3] - 5;

        if (supplies[3] < 0)
        {
            std::cout << "YOU DIE OF SNAKEBITE SINCE YOU HAVE NO MEDICINE" << std::endl;
            return true;
        }
    }
    else if (randomNum <= 54)
    {
        std::cout << "WAGON GETS SWAMPED FORDING RIVER--LOSE FOOD AND CLOTHES" << std::endl;
        supplies[0] = supplies[0] - 30;
        supplies[2] = supplies[2] - 20;
        totalMileage -= 20 - 20 * smallRandom();
    }
    else if (randomNum <= 64)
    {
        float reactionTime = 0.0f;
        std::cout << "WILD ANIMALS ATTACK!" << std::endl;
        reactionTime = shooting(shootingDiff);

        if (supplies[1] < 40)
        {
            std::cout << "YOU WERE TOO LOW ON BULLETS--" << std::endl;
            std::cout << "THE WOLVES OVERPOWERED YOU" << std::endl;
            flags[1] = 1;
            gameOver(DeathType::INJURIES);

            result = true; //ded
        }
        else if (reactionTime > 2.0f)
        {
            std::cout << "SLOW ON THE DRAW---THEY GOT AT YOUR FOOD AND CLOTHES" << std::endl;
            supplies[0] = supplies[0] - reactionTime * 8; //food
            supplies[1] = supplies[1] - 20 * reactionTime; //bullets
            supplies[2] = supplies[2] - reactionTime * 4; //clothes
        }
        else
        {
            std::cout << "NICE SHOOTIN' PARDNER---THEY DIDN'T GET MUCH" << std::endl;
            supplies[1] = supplies[1] - 20 * reactionTime; //bullets
        }
    }
    else if (randomNum <= 69)
    {
        std::cout << "HAIL STORM---SUPPLIES DAMAGED" << std::endl;
        totalMileage -= 5 - smallRandom() * 10;
        supplies[1] = supplies[1] - 200;
        supplies[3] = supplies[3] - 4 - smallRandom() * 3;
    }
    else if (randomNum <= 95)
    {
        float randomNum = smallRandom();
        if ((eatingPref == 1) || (eatingPref == 2 && randomNum < 0.25f) || (eatingPref == 3 && randomNum < 0.5f))
            result = illnessDisplay(supplies);
    }
    else
    {
        std::cout << "HELPFUL INDIANS SHOW YOU WERE TO FIND MORE FOOD" << std::endl;
        supplies[0] = supplies[0] + 14;
    }


    return result;
}

//Return whether alive
bool eating(short *supplies, unsigned short shootingDiff)
{
    //Eating
    unsigned short hostilityFactor = 0; //S5
    short input = 0; //E/T1
    bool canAfford = 0;

    if (supplies[0] <= 0)
    {
        gameOver(DeathType::NO_FOOD);
        return true;
    }

    for (;;)
    {
            std::cout << "DO YOU WANT TO EAT (1) POORLY   (2) MODERATELY   OR  (3) WELL" << std::endl;
            eatingPref = getInput();

            canAfford = (supplies[0] - 8 - 5 * eatingPref) >= 0;
            if (!canAfford)
                std::cout << "YOU CAN'T EAT THAT WELL" << std::endl;
            else if (!inRange(eatingPref, 1, 3))
                std::cout << "INPUT NOT IN RANGE" << std::endl;
            else
                break;
    }

    supplies[0] = supplies[0] - 8 - (5 * eatingPref);
    totalMileage += 200 + (numOfOxen - 220) / 5 + 10 * smallRandom();

    flags[6] = flags[7] = 0;

    //Rider attacks
    if ((smallRandom() * 10) <= 600000.0f / (float)(totalMileage ^ 2 - 800 * totalMileage + 280000))
    {
        std::cout << "RIDERS AHEAD.   THEY ";

        if (smallRandom() >= 0.8f)
        {
            std::cout << "DON'T ";
            hostilityFactor = 1;
        }

        std::cout << "LOOK HOSTILE" << std::endl;
        std::cout << "TACTICS" << std::endl;

        if (smallRandom() <= 0.2f)
            hostilityFactor = 1 - hostilityFactor;

        for (;;)
        {
            std::cout << "(1) RUN  (2) ATTACK  (3) CONTINUE  (4) CIRCLE WAGONS" << std::endl;
            input = getInput();

            if (inRange(input, 1, 4))
                break;
        }

        //If people are hostile
        if (hostilityFactor == 0)
        {
            switch (input) {
                case 1: //Run
                    totalMileage += 20;
                    supplies[1] = supplies[1] - 150; //Lower bullets
                    supplies[3] = supplies[3] - 15; //Lower misc supplies
                    numOfOxen -= 40; //Lower oxen
                    break;
                case 2: //Attack
                    banditDefenseEval(supplies, shootingDiff);
                    break;
                case 3: //Continue
                    if (smallRandom() > 0.8f)
                        std::cout << "THEY DID NOT ATTACK" << std::endl;
                    else
                    {
                        supplies[1] = supplies[1] - 150;
                        supplies[3] = supplies[3] - 15;
                    }
                    break;
                case 4: //Circle wagons
                    totalMileage -= 25;

                    banditDefenseEval(supplies, shootingDiff);
                    break;
            }
        }
        //If people are friendly
        else
        {
            switch (input) {
                case 1: //Run
                    totalMileage += 15;
                    numOfOxen -= 10;
                    break;
                case 2: //Attack
                    totalMileage -= 5;
                    supplies[1] = supplies[1] - 100;
                case 3: //Continue
                    break;
                case 4: //Circle wagons
                    totalMileage -= 20;
                    break;
            }
        }

        //Outcome
        if (hostilityFactor == 1)
            std::cout << "RIDERS WERE FRIENDLY, BUT CHECK FOR POSSIBLE LOSSES" << std::endl;
        else
        {
            std::cout << "RIDERS WERE HOSTILE--CHECK FOR LOSSES" << std::endl;

            if (supplies[1] < 0)
            {
                std::cout << "YOU RAN OUT OF BULLETS AND GOT MASSACRED BY THE RIDERS" << std::endl;
                return true;
            }
        }
    }

    return false;
}

/**
 * Handles purchasing an item a fort
 */
unsigned short processSale(const char *item, short *cashLeft)
{
    unsigned short input = 0;
    short userInput = 0;

    std::cout << item << std::endl;
    userInput = getInput();
    input = userInput > 0 ? userInput : 0;
    

    if (input >= 0)
    {
        if (*cashLeft < 0)
        {
            std::cout << "YOU DON'T HAVE THAT MUCH--KEEP YOUR SPENDING DOWN\n"
                << "YOU MISS YOUR CHANCE TO SPEND ON THAT ITEM" << std::endl;

            input = 0;
        }
        else
            *cashLeft -= input;
    }
    else
        input = 0;

    return input;
}

/**
 * Handles being at a fort
 */
bool goToFort(short *supplies, short *cashLeft, unsigned short shootingDiff)
{
    bool result = false;
    unsigned short input = 0;
    std::cout << "ENTER WHAT YOU WISH TO SPEND ON THE FOLLOWING" << std::endl;

    input = processSale("FOOD", cashLeft);
    supplies[0] = supplies[0] + (2.0f / 3.0f) * (float)input;

    input = processSale("AMMUNITION", cashLeft);
    supplies[1] = supplies[1] + (2.0f / 3.0f) * (float)input * 50.0f;

    input = processSale("CLOTHING", cashLeft);
    supplies[2] = supplies[2] + (2.0f / 3.0f) * (float)input;

    input = processSale("MISCELLANEOUS SUPPLIES", cashLeft);
    supplies[3] = supplies[3] + (2.0f / 3.0f) * (float)input;

    totalMileage -= 45;

    if (supplies[0] < 13)
        result = true; //dead
    else
        result = eating(supplies, shootingDiff);

    return result;
}

/**
 * Handles hunting
 */
bool goHunting(short *supplies, unsigned short shootingDiff)
{
    bool result = false;
    float reactionTime = 0.0f;
    float randomNum = 100.0f * smallRandom();
    totalMileage -= 45;

    reactionTime = shooting(shootingDiff);

    if (reactionTime < 1.0f) //Perfection
    {
        std::cout << "RIGHT BETWEEN THE EYES---YOU GOT A BIG ONE!!!!"
            << "FULL BELLIES TONIGHT!" << std::endl;

        supplies[0] = supplies[0] + 52 + smallRandom() * 6.0f;
        supplies[1] = supplies[1] - 10 - smallRandom() * 4.0f;
    }
    else if (randomNum < (13.0f * reactionTime)) //Missed
        std::cout << "YOU MISSED---AND YOUR DINNER GOT AWAY....." << std::endl;
    else //Did okay (passed)
    {
        supplies[0] = supplies[0] + 48 - 2 * reactionTime;
        std::cout << "NICE SHOT--RIGHT ON TARGET--GOOD EATIN' TONIGHT!!" << std::endl;
        supplies[1] = supplies[1] - 10 - 3 * reactionTime;
    }

    if (supplies[0] < 13)
    {
        result = true; //dead
        gameOver(DeathType::NO_FOOD);
    }
    else
        result = eating(supplies, shootingDiff);

    return result;
}

/**
 * Prints the current date (if too late in time, they die)
 */
bool printDate(unsigned short *turnNumber)
{
    std::cout << std::endl << "MONDAY ";
    
    if (*turnNumber < 18)
    {
        std::cout << DATES[*turnNumber] << "1847" << std::endl << std::endl;
        (*turnNumber)++;
        return false;
    }
    else
    {
        gameOver(DeathType::TOO_LATE);
        return true;
    }
}

/**
 * Handles illness/injury, displaying current status, and getting input for initial turn options
 */
ChoiceEnd* processTurn(short *supplies, short *cashLeft, unsigned short shootingDiff)
{
    ChoiceEnd* choices = new ChoiceEnd;
    choices->dead = false;

    for (int i = 0; i < 4; ++i)
    {
        if (supplies[i] < 0)
            supplies[i] = 0;
    }

    if (supplies[0] < 13)
        std::cout << "YOU'D BETTER DO SOME HUNTING OR BUY FOOD AND SOON!!!!" << std::endl;

    if (flags[1] || flags[2]) //Injury or Illness
    {
        *cashLeft -= 20;

        if (*cashLeft < 0) //No money to pay for doctor, die
        {
            choices->choice = -1;
            choices->dead = true;
            gameOver(DeathType::CANT_PAY_DOC);

            return choices;
        }

        std::cout << "DOCTOR'S BILL IS $20" << std::endl;

        flags[1] = flags[2] = 0;
    }

    //Displays status
    if (flags[5])
    {
        std::cout << "TOTAL MILEAGE IS 950" << std::endl;
        flags[5] = 0;
    }
    else
        std::cout << "TOTAL MILEAGE IS " << totalMileage << std::endl;

    std::cout << "FOOD\tBULLETS\tCLOTHES\tMISCSUP\tCASH" << std::endl;
    for (int i = 0; i < 4; ++i)
        std::cout << supplies[i] << "\t";
    std::cout << *cashLeft << std::endl;

    if (flags[0] == 1) //Fort option is on
    {
        flags[0] == 0;

        std::cout << "DO YOU WANT TO (1) STOP AT THE NEXT FORT, (2) HUNT, OR (3) CONTINUE" << std::endl;
        choices->choice = getInput();

        if ((choices->choice < 1) || (choices->choice > 2))
            choices->choice = 3;
    }
    else //No fort option
    {
        std::cout << "DO YOU WANT TO (1) HUNT, OR (2) CONTINUE" << std::endl;
        choices->choice = getInput();

        if (choices->choice == 1)
            choices->choice = 2;
        else
            choices->choice = 3;

        if ((choices->choice == 3) || (supplies[1] > 39))
            flags[0] = 1;
        else if ((choices->choice == 2) && (supplies[1] <= 39))
        {
            std::cout << "TOUGH YOU NEED MORE BULLETS TO GO HUNTING" << std::endl;
            choices->choice = 3;
        }
    }

    return choices;
}

int main()
{
    ChoiceEnd *info = nullptr;
    short supplies[] = { 0, 0, 0, 0 }; //F, B, C, M1
    unsigned short turnNumber = 0, previousMileage = 0; //D3, M2
    short cashLeft = 0, shootingDiff = 0; //T, D9
    char boolResponse = '\0'; //C$
    const std::string supplyNames[] = {
        "FOOD",
        "AMMUNITION",
        "CLOTHING",
        "MISCELLANEOUS SUPPLIES"
    };
    bool dead = false, started = false; //custom

    srand(time(NULL));
    std::cout << "DO YOU NEED INSTRUCTIONS  (y/n)" << std::endl;

    std::cin >> boolResponse;
    if (boolResponse == 'y' || boolResponse == 'Y')
    {
        std::cout << "\n\nTHIS PROGRAM SIMULATES A TRIP OVER THE OREGON TRAIL FROM\n"
            << "INDEPENDENCE, MISSOURI TO OREGON CITY, OREGON IN 1847.\n"
            << "YOUR FAMILY OF FIVE WILL COVER THE 2040 MILE OREGON TRAIL\n"
            << "IN 5-6 MONTHS --- IF YOU MAKE IT ALIVE.\n" << std::endl

            << "YOU HAD SAVED $900 TO SPEND FOR THE TRIP, AND YOU'VE JUST\n"
            << "   PAID $200 FOR A WAGON .\n"
            << "YOU WILL NEED TO SPEND THE REST OF YOUR MONEY ON THE\n"
            << "   FOLLOWING ITEMS:\n" << std::endl

            << "     OXEN - YOU CAN SPEND $200-$300 ON YOUR TEAM\n"
            << "            THE MORE YOU SPEND, THE FASTER YOU'LL GO\n"
            << "               BECAUSE YOU'LL HAVE BETTER ANIMALS\n" << std::endl

            << "     FOOD - THE MORE YOU HAVE, THE LESS CHANCE THERE\n"
            << "               IS OF GETTING SICK\n" << std::endl

            << "     AMMUNITION - 81 BUYS A BELT OF 50 BULLETS\n"
            << "     AMMUNITION - $1 BUYS A BELT OF 50 BULLETS\n"
            << "            YOU WILL NEED BULLETS FOR ATTACKS BY ANIMALS\n"
            << "               AND BANDITS, AND FOR HUNTING FOOD\n" << std::endl

            << "     CLOTHING - THIS IS ESPECIALLY IMPORTANT FOR THE COLD\n"
            << "               WEATHER YOU WILL ENCOUNTER WHEN CROSSING\n"
            << "               THE MOUNTAINS\n" << std::endl

            << "     MISCELLANEOUS SUPPLIES - THIS INCLUDES MEDICINE AND\n"
            << "               OTHER THINGS YOU WILL NEED FOR SICKNESS\n"
            << "               AND EMERGENCY REPAIRS\n\n" << std::endl


            << "YOU CAN SPEND ALL YOUR MONEY BEFORE YOU START YOUR TRIP -\n"
            << "OR YOU CAN SAVE SOME OF YOUR CASH TO SPEND AT FORTS ALONG\n"
            << "THE WAY WHEN YOU RUN LOW. H0WEVER, ITEMS COST MORE AT\n"
            << "THE FORTS. YOU CAN ALSO GO HUNTING ALONG THE WAY TO GET\n"
            << "MORE FOOD.\n"
            << "WHENEVER YOU HAVE TO USE YOUR TRUSTY RIFLE ALONG THE WAY,\n"
            << "YOU WILL BE TOLD TO TYPE IN A WORD (ONE THAT SOUNDS LIKE A\n"
            << "GUN SHOT). THE FASTER YOU TYPE IN THAT WORD AND HIT THE\n"
            << "**RETURN** KEY, THE BETTER LUCK YOU'LL HAVE WITH YOUR GUN.\n" << std::endl

            << "AT EACH TURN, ALL ITEMS ARE SHOWN IN DOLLAR AMOUNTS\n"
            << "EXCEPT BULLETS\n"
            << "WHEN ASKED TO ENTER MONEY AMOUNTS, DON'T USE A **$**.\n\n"

            << "GOOD LUCK!!!" << std::endl;
    }

    std::cout << "\n\nHOW GOOD A SHOT ARE YOU WITH YOUR RIFLE?\n"
        << "  (1) ACE MARKSMAN,  (2) GOOD SHOT,  (3) FAIR TO MIDDLIN'\n"
        << "         (4) NEED MORE PRACTICE,  (5) SHAKY KNEES\n"
        << "ENTER ONE OF THE ABOVE -- THE BETTER YOU CLAIM YOU ARE, THE\n"
        << "FASTER YOU'LL HAVE TO BE WITH YOUR GUN TO BE SUCCESSFUL." << std::endl;

    shootingDiff = getInput();
    if (shootingDiff > 5)
        shootingDiff = 0;

    for (;;)
    {
        started = getInitialStats(supplies, supplyNames, &cashLeft);

        if (started)
            break;
        else
            std::cout << "YOU OVERSPENT--YOU ONLY HAD $700 TO SPEND.  BUY AGAIN." << std::endl;
    }

    std::cout << "AFTER ALL YOUR PURCHASES, YOU NOW HAVE " << cashLeft << " DOLLARS LEFT\n\n"
        << "MONDAY MARCH 29 1847\n" << std::endl;

    supplies[1] = supplies[1] * 50;

    //Handles the main game loop
    do
    {
        previousMileage = totalMileage;
        info = processTurn(supplies, &cashLeft, shootingDiff);
        dead = info->dead;

        if (!dead && flags[8] != 1)
        {
            if (info->choice == 1)
                dead = goToFort(supplies, &cashLeft, shootingDiff);
            else if (info->choice == 2)
                dead = goHunting(supplies, shootingDiff);
            else
                dead = eating(supplies, shootingDiff);
        }

        if (!dead && flags[8] != 1)
            dead = selectionOfEvents(supplies, shootingDiff, &cashLeft);

        if (!dead && flags[8] != 1)
            dead = mountainProcessing(supplies);

        if (!dead && flags[8] != 1 && totalMileage < 2040)
            dead = printDate(&turnNumber);

        info->dead = dead;

    } while (!dead && totalMileage < 2040);

    //Died!
    if (dead)
    {
        std::cout << "\nDUE TO YOUR UNFORTUNATE SITUATION, THERE ARE A FEW\n"
            << "FORMALITIES WE MUST GO THROUGH\n" << std::endl;

        std::cout << "WOULD YOU LIKE A MINISTER?" << std::endl;
        std::cin >> boolResponse;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "WOULD YOU LIKE A FANCY FUNERAL?" << std::endl;
        std::cin >> boolResponse;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "WOULD YOU LIKE US TO INFORM YOUR NEXT OF KIN?" << std::endl;
        std::cin >> boolResponse;

        switch (boolResponse)
        {
        case 'y':
        case 'Y':
            std::cout << "THAT WILL BE $4.50 FOR THE TELEGRAPH CHARGE.\n" << std::endl;
            break;
        case 'n':
        case 'N':
            std::cout << "BUT YOUR AUNT SADIE IN ST. LOUIS IS REALLY WORRIED ABOUT YOU\n" << std::endl;
            break;
        }

        std::cout << "WE THANK YOU FOR THIS INFORMATION AND WE ARE SORRY YOU\n"
            << "DIDN'T MAKE IT TO THE GREAT TERRITORY OF OREGON\n"
            << "BETTER LUCK NEXT TIME\n\n\n"
            << "                              SINCERELY\n\n"
            << "                 THE OREGON CITY CHAMBER OF COMMERCE" << std::endl;


    }
    //Game Won!
    else if (flags[8] == 1 || totalMileage >= 2040)
    {
        unsigned short dayOfWeek = 0;
        float fractOfFortnight = ((float)(2040.0f - previousMileage) / (totalMileage - previousMileage));
        supplies[0] += (1 - fractOfFortnight) * (8 * 5 * eatingPref);
 
        std::cout << "\nYOU FINALLY ARRIVED AT OREGON CITY\n"
        << "AFTER 2040 LONG MILES---HOORAY !!!!!\n"
        "A REAL PIONEER!\n" << std::endl;
        
        dayOfWeek = (fractOfFortnight * 14);
        turnNumber = turnNumber * 14 + dayOfWeek; //Number of in-world days on the trail

        if (dayOfWeek > 6)
            dayOfWeek -= 7;

        std::cout << DAYS_OF_WEEK[dayOfWeek];

        if (turnNumber < 125)
        {
            turnNumber -= 93;
            std::cout << "JULY " << turnNumber << " 1847" << std::endl;
        }
        else if (turnNumber < 156)
        {
            turnNumber -= 124;
            std::cout << "AUGUST " << turnNumber << " 1847" << std::endl;
        }
        else if (turnNumber < 186)
        {
            turnNumber -= 155;
            std::cout << "SEPTEMBER " << turnNumber << " 1847" << std::endl;
        }
        else if (turnNumber < 217)
        {
            turnNumber -= 185;
            std::cout << "OCTOBER " << turnNumber << " 1847" << std::endl;
        }
        else if (turnNumber < 247)
        {
            turnNumber -= 216;
            std::cout << "NOVEMBER " << turnNumber << " 1847" << std::endl;
        }
        else
        {
            turnNumber -= 246;
            std::cout << "DECEMBER " << turnNumber << " 1847" << std::endl;
        }

        std::cout << "\nFOOD\tBULLETS\tCLOTHES\tMISCSUP\tCASH" << std::endl;
        for (int i = 0; i < 4; ++i)
        {
            if (supplies[i] < 0)
                supplies[i] = 0;
            std::cout << supplies[i] << '\t';
        }
        if (cashLeft < 0) 
            cashLeft = 0;
        std::cout << cashLeft << std::endl << std::endl;

        std::cout << "PRESIDENT JAMES K. POLK SENDS YOU HIS\n"
            << "      HEARTIEST CONGRATULATIONS\n\n"

            << "AND WISHES YOU A PROSPEROUS LIFE AHEAD\n\n"

            << "           AT YOUR NEW HOME" << std::endl;
    }

    delete[] info;

    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.ignore(10, '\n');
    std::cin.get();

    return 0;
}
