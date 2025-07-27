#pragma once
#include "core/object/object.h"
#include <functional>
// TSpawnType = Object 子类
template<typename TSpawnType>
class Spawner : public Object {
protected:
    int num_ = 20;				// 每次生成的数量
    int max_num_ = 100;			// 最大生成数量 todo:达到最大生成数量就停止生成
    float interval_timer_ = 0;			// 计时器
    float interval_time_ = 3.0f;		// 生成间隔时间

	// 工厂：保存生产方法
    std::function<TSpawnType* (void)> doSpawn_;
public:
    static Spawner<TSpawnType>* addSpawnerChild(Object* parent, int num, float interval_time, 
        std::function<TSpawnType*(void)> doSpawn, int max_num = -1) 
    {
        Spawner<TSpawnType>* spawner = new  Spawner<TSpawnType>;
        spawner->num_ = num;
        spawner->max_num_ = max_num;
        spawner->interval_time_ = interval_time;
        spawner->doSpawn_ = doSpawn;
        if (parent) parent->addChild(spawner);
        return spawner;
    };


    virtual void update(float dt) override {

		// 构造 T 类型的对象，加入到当前场景中  todo:达到最大生成数量就停止生成
		interval_timer_ += dt;
		if (interval_timer_ >= interval_time_)
		{
			interval_timer_ = 0;
            for (int i = 0;i < num_;i++) {
                TSpawnType* obj = doSpawn_();
                if(obj) engine_.getCurrentScene()->safeAddChild(obj);
            } 
		}

    };
};
