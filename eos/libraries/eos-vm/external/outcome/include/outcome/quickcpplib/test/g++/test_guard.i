






namespace boost { namespace afio { inline namespace v1_std_boost_asio {

static const char *boost_bindlib_out[]={ "BOOST_AFIO_V1", "BOOST_AFIO_USE_BOOST_THREAD=0 BOOST_AFIO_USE_BOOST_FILESYSTEM=1 ASIO_STANDALONE=1" };

int foo()
{
  return 1;
}

} } }

static int (*abi1)()=boost :: afio :: v1_std_boost_asio::foo;




namespace boost { namespace afio { inline namespace v1_boost_std_boost {

static const char *boost_bindlib_out[]={ "BOOST_AFIO_V1", "BOOST_AFIO_USE_BOOST_THREAD=1 BOOST_AFIO_USE_BOOST_FILESYSTEM=0 ASIO_STANDALONE=0" };

int foo()
{
  return 1;
}

} } }

static int (*abi2)()=boost :: afio :: v1_boost_std_boost::foo;





static int (*abi3)()=boost :: afio :: v1_std_boost_asio::foo;


int main(void)
{
  return abi1!=abi3

  || abi1==abi2 || abi2==abi3

  ;
}
