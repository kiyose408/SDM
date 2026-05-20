#pragma once

#include <QObject>

namespace smart_diet {

/**
 * @brief TDEE/BMR 计算器 — Mifflin-St Jeor 公式
 *
 * 来源：005.接口设计文档 §3.1
 *
 * 公式：
 *   男性 BMR = 10×W + 6.25×H - 5×A + 5
 *   女性 BMR = 10×W + 6.25×H - 5×A - 161
 *   TDEE = BMR × activityFactor
 *
 * 其中：W=体重(kg), H=身高(cm), A=年龄(岁)
 *       默认 activityFactor = 1.55 (中等活动量，来自 constants.h)
 */
class TdeeCalculator : public QObject {
    Q_OBJECT

public:
    explicit TdeeCalculator(QObject *parent = nullptr);

    /**
     * @brief 计算 BMR
     * @param gender  1=男, 2=女
     * @param weight  体重 (kg)
     * @param height  身高 (cm)
     * @param age     年龄 (岁)
     * @return BMR (kcal/day)，无效输入返回 0
     */
    Q_INVOKABLE double calculateBmr(int gender, double weight, double height, int age) const;

    /**
     * @brief 计算 TDEE
     * @param bmr             BMR 值
     * @param activityFactor  活动系数（默认 1.55）
     * @return TDEE (kcal/day)
     */
    Q_INVOKABLE double calculateTdee(double bmr, double activityFactor = 1.55) const;

    /**
     * @brief 结合饮食目标修正 TDEE
     * @param tdee     TDEE 值
     * @param dietGoal 0=减脂, 1=增肌, 2=维持
     * @return 修正后 TDEE
     *
     * 系数：减脂 ×0.80, 增肌 ×1.15, 维持 ×1.00
     */
    Q_INVOKABLE double applyDietGoal(double tdee, int dietGoal) const;
};

} // namespace smart_diet
