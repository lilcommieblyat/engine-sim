#include "../include/ignition_module.h"

#include "../include/utilities.h"
#include "../include/constants.h"
#include "../include/units.h"

#include <cmath>

IgnitionModule::IgnitionModule() {
    m_plugs = nullptr;
    m_crankshaft = nullptr;
    m_timingCurve = nullptr;
    m_cylinderCount = 0;
    m_lastCrankshaftAngle = 0.0;
}

IgnitionModule::~IgnitionModule() {
    assert(m_plugs == nullptr);
}

void IgnitionModule::destroy() {
    delete[] m_plugs;
    m_cylinderCount = 0;
}

void IgnitionModule::initialize(const Parameters &params) {
    m_cylinderCount = params.CylinderCount;
    m_plugs = new SparkPlug[m_cylinderCount];
    m_crankshaft = params.Crankshaft;
    m_timingCurve = params.TimingCurve;
}

void IgnitionModule::setFiringOrder(int cylinderIndex, double angle) {
    assert(cylinderIndex < m_cylinderCount);

    m_plugs[cylinderIndex].Angle = angle;
}

void IgnitionModule::reset() {
    m_lastCrankshaftAngle = m_crankshaft->getCycleAngle();
    resetIgnitionEvents();
}

void IgnitionModule::update(double dt) {
    if (m_crankshaft->m_body.v_theta >= 0) return;

    const double cycleAngle = m_crankshaft->getCycleAngle();
    const double fourPi = 4 * Constants::pi;
    const double advance = getTimingAdvance();

    for (int i = 0; i < m_cylinderCount; ++i) {
        double adjustedAngle = positiveMod(m_plugs[i].Angle - advance, fourPi);
        const double r0 = m_lastCrankshaftAngle;
        double r1 = cycleAngle;

        if (cycleAngle < r0) {
            r1 += fourPi;
            adjustedAngle += fourPi;
        }

        if (adjustedAngle >= r0 && adjustedAngle < r1) {
            m_plugs[i].IgnitionEvent = true;
        }
    }

    m_lastCrankshaftAngle = cycleAngle;
}

bool IgnitionModule::getIgnitionEvent(int index) const {
    return m_plugs[index].IgnitionEvent;
}

void IgnitionModule::resetIgnitionEvents() {
    for (int i = 0; i < m_cylinderCount; ++i) {
        m_plugs[i].IgnitionEvent = false;
    }
}

double IgnitionModule::getTimingAdvance() {
    return m_timingCurve->sampleTriangle(-m_crankshaft->m_body.v_theta);
}

IgnitionModule::SparkPlug *IgnitionModule::getPlug(int i) {
    while (i < 0) i += m_cylinderCount;
    return &m_plugs[i % m_cylinderCount];
}