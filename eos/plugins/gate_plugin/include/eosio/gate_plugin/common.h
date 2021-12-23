
#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <cassert>

using namespace std;

constexpr size_t cx_hash(const char* input){
    size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
    const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

    while (*input) {
        hash ^= static_cast<size_t>(*input);
        hash *= prime;
        ++input;
    }
    return hash;
}

constexpr size_t cx_hash(const std::string& input){
    size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
    const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

    const char* i = input.c_str();
    while (*i) {
        hash ^= static_cast<size_t>(*i);
        hash *= prime;
        ++i;
    }
    return hash;
}

 /* Basic Usage:
        template<typename MessageType>
        class CMouse : public patterns::CSupervisor<MessageType>
        {
            private:
                unsigned int m_iPosX;
                unsigned int m_iPosY;

            public:
                CMouse( ) : m_iPosX( 0 ), m_iPosY( 0 ) {};
                ~CMouse( ) {};

                void StateChanged( )
                {
                    DoNotifyAll( 666 );
                };
        };

        template<typename MessageType>
        class CWidget : public patterns::IObserver<MessageType>
        {
            public:
                CWidget( ) {};
                virtual ~CWidget( ) {};

                virtual void HandleEvent( const MessageType& inMessage )
                {
                    std::cout << " incomming message to CWidget - " << inMessage << std::endl; 
                };
        };

        template<typename MessageType>
        class CWindow : public patterns::IObserver<MessageType>
        {
            public:
                CWindow( ) {};
                virtual ~CWindow( ) {};

                virtual void HandleEvent( const MessageType& inMessage )
                {
                    std::cout << " incomming message to CWindow - " << inMessage << std::endl; 
                };
        };
    */

template<typename Type>
class IObserver{
    public:
        virtual void HandleEvent( const Type& ) = 0;
};

template<typename Type>
class CSupervisor{
    protected:
        std::vector<IObserver<Type>*> m_Subscribers;

    public:
        CSupervisor( ) : m_Subscribers( ){            
        };

        virtual ~CSupervisor( ){
            DoClear( );
        };

        void DoClear( ){
            m_Subscribers.clear( );
        };

        void DoAddSubscriber( IObserver<Type>* const inReference ){
            m_Subscribers.push_back( inReference );
        };
 
        void DoRemoveSubscriber( IObserver<Type>* const inReference ){
            m_Subscribers.erase( inReference );
        };

        void DoNotifyAll( const Type& inMessage ){
		    for( unsigned int i = 0; i < m_Subscribers.size( ); ++i )
			    m_Subscribers[i]->HandleEvent( inMessage );
        };
};

template<typename Type>
class CDeferredSupervisor : public CSupervisor<Type>{
	private:
		std::vector<Type> m_vMessageQueue;

	public:
		CDeferredSupervisor( ) : CSupervisor<Type>( ){           
        };

		virtual ~CDeferredSupervisor( ){            
        };

		void DoPushBackEvent( const Type& inMessage ){
			m_vMessageQueue.push_back( inMessage );
		};

		uint32_t GetQueueSize( ) const{
			return m_vMessageQueue.size( );
		};

		bool IsEmpty( ) const{
			return !m_vMessageQueue.size( );
		};

		void DoClearEvents( ){
			m_vMessageQueue.clear( );
		};

		void DoProcessEvents( ){
			for( uint32_t i = 0; i < CSupervisor<Type>::m_Subscribers.size( ); ++i )
				for( uint32_t z = 0; z < m_vMessageQueue.size( ); ++z )
					 CSupervisor<Type>::m_Subscribers[i]->HandleEvent( m_vMessageQueue[z] );

			DoClearEvents( );
		};
};