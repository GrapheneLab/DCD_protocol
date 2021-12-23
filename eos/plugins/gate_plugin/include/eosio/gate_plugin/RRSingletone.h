#pragma once

template<typename Type>
class RRingleton{
public:
	static Type* GetInstance(){
		if (!m_Instance)
			m_Instance = new Type();

		return m_Instance;
	};

	static void DeleteInstance(){
		if (m_Instance){
			delete m_Instance;
			m_Instance = NULL;
		}
	};

protected:
	RRingleton() {
	};
	virtual ~RRingleton() {
	};

private:
	RRingleton(const RRingleton& inData) {
	};
	static Type* m_Instance;
};
template<typename Type> Type* RRingleton<Type>::m_Instance = NULL;
