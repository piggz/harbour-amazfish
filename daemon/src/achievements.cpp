#include "achievements.h"

#ifdef UUITK_EDITION
#include <libusermetricsinput/MetricManager.h>
using namespace UserMetricsInput;
#endif

Achievements::Achievements(QObject *parent)
    : QObject(parent)
{
    loreStepMessages = {
        {0.0,  tr("The journey begins, adventurer. Only <b>%1</b> steps — the Shire is still in sight.")},
        {0.25, tr("You’ve crossed the Brandywine. <b>%1</b> steps down — trolls ahead!")},
        {0.5,  tr("Halfway to Mordor. <b>%1</b> steps walked and second breakfast missed.")},
        {0.8,  tr("Mount Doom is on the horizon. <b>%1</b> steps behind you — don't drop the ring yet!")},
        {1.0,  tr("The ring is cast into the fire. <b>%1</b> steps done — Middle-earth is saved!")},
        {1.2,  tr("You kept walking after saving the world?! <b>%1</b> steps — you’re a true legend. 🧙")},
        {2.0,  tr("The Hobbit, or There and Back Again — <b>%1</b> steps and second breakfast earned!")}
    };

     stepMessages = {
        {0.0,  tr("Let's get moving! You've taken only <b>%1</b> steps so far.")},
        {0.25, tr("Warming up! <b>%1</b> steps done — keep going!")},
        {0.5,  tr("You're halfway there. <b>%1</b> steps so far!")},
        {0.8,  tr("Almost there! Just a bit more — <b>%1</b> steps already!")},
        {1.0,  tr("Goal reached! <b>%1</b> steps — you can have that cake now 🎉")},
        {1.2,  tr("You smashed it! <b>%1</b> steps — time to rest or go for bonus steps?")}
    };
}

void Achievements::updateStepsStatus(int steps, int goal)
{

#ifdef UUITK_EDITION
    try {
	MetricManagerPtr manager(MetricManager::getInstance());
	MetricPtr metric(
	    manager->add(
		MetricParameters("uk.co.piggz.harbour-amazfish.steps-metric")
		    .formatString(selectStepsMessage(steps, goal))
		    .emptyDataString("No data")
		    .textDomain("harbour-amazfish")
		)
	    );
	metric->update(static_cast<double>(steps));
	qDebug() << "metric-update(double steps)" << steps;
    } catch (const std::exception& e) {
	qWarning() << "Failed to update steps metric:" << e.what();
    }
#endif

}

QString Achievements::selectStepsMessage(int steps, double goal)
{
    QDate today = QDate::currentDate();
    std::map<float, QString> usedMap;

    // use lord of rings messages whole march
    usedMap = (today.month() == 3) ? loreStepMessages : stepMessages;

    double reached = (goal > 0) ? steps / static_cast<double>(goal) : 0.0;

    // qDebug() << Q_FUNC_INFO  << reached << " " << steps << " " << goal;

    for (const auto& entry : usedMap) {
        if (entry.first > reached) {
            return entry.second.arg(steps);
        }
    }

    return usedMap.rbegin()->second.arg(steps);
}
