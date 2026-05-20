#include "tdee_calculator.h"

namespace smart_diet {

TdeeCalculator::TdeeCalculator(QObject *parent)
    : QObject(parent)
{
}

double TdeeCalculator::calculateBmr(int gender, double weight,
                                     double height, int age) const
{
    if (weight <= 0 || height <= 0 || age <= 0) return 0.0;
    if (gender < 1 || gender > 2) return 0.0;

    // Mifflin-St Jeor 公式
    double bmr = 10.0 * weight + 6.25 * height - 5.0 * age;
    if (gender == 1) {
        bmr += 5.0;     // 男性
    } else {
        bmr -= 161.0;   // 女性
    }
    return bmr;
}

double TdeeCalculator::calculateTdee(double bmr, double activityFactor) const
{
    if (bmr <= 0) return 0.0;
    return bmr * activityFactor;
}

double TdeeCalculator::applyDietGoal(double tdee, int dietGoal) const
{
    switch (dietGoal) {
    case 0: return tdee * 0.80;   // 减脂
    case 1: return tdee * 1.15;   // 增肌
    case 2: return tdee;          // 维持（不修正）
    default: return tdee;
    }
}

} // namespace smart_diet
