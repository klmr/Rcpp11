#ifndef Rcpp__vector__Vector_h
#define Rcpp__vector__Vector_h

namespace Rcpp{

template <
    int RTYPE, 
    template <class> class StoragePolicy
>
class Vector :
    public VectorBase< RTYPE, true, Vector<RTYPE,StoragePolicy> >,
    public StoragePolicy<Vector<RTYPE,StoragePolicy>>,
    public SlotProxyPolicy<Vector<RTYPE,StoragePolicy>>,
    public AttributeProxyPolicy<Vector<RTYPE,StoragePolicy>>,
    public NamesProxyPolicy<Vector<RTYPE, StoragePolicy>>,
    public AttributesProxyPolicy<Vector<RTYPE, StoragePolicy>>, 
    public RObjectMethods<Vector<RTYPE, StoragePolicy>>
{
public:
    typedef typename traits::r_vector_proxy<RTYPE>::type Proxy ;
    typedef typename traits::r_vector_const_proxy<RTYPE>::type const_Proxy ;
    typedef typename traits::r_vector_name_proxy<RTYPE>::type NameProxy ;
    typedef typename traits::r_vector_const_name_proxy<RTYPE>::type const_NameProxy ;
    typedef typename traits::r_vector_proxy<RTYPE>::type value_type ;
    typedef typename traits::r_vector_iterator<RTYPE>::type iterator ;
    typedef typename traits::r_vector_const_iterator<RTYPE>::type const_iterator ;
    typedef typename traits::init_type<RTYPE>::type init_type ;
    typedef typename traits::r_vector_element_converter<RTYPE>::type converter_type ;
    typedef typename traits::storage_type<RTYPE>::type stored_type ;

    RCPP_GENERATE_CTOR_ASSIGN(Vector)

    Vector( SEXP x ) {
        Storage::set__( r_cast<RTYPE>( x ) ) ;
    }
    
    Vector( int n ) : Vector(Rf_allocVector(RTYPE, n) ) {}
    Vector() : Vector(0) {}

    template <typename U>
    Vector( int n, const U& u ) : Vector(n) {
        fill( u ) ;
    }
    
    Vector( const std::string& st ) : Vector( internal::vector_from_string<RTYPE>(st) ) {}
    Vector( const char* st ) : Vector(internal::vector_from_string<RTYPE>(st)){}

    template <bool NA, typename VEC>
    Vector( const SugarVectorExpression<RTYPE,NA,VEC>& other ) : Vector(other.size()) {
        other.apply(*this) ;
    }

    Vector( std::initializer_list<init_type> list ) {
        Storage::set__( r_cast<RTYPE>( wrap( list.begin(), list.end() ) ) );
    }

    template <bool NA, typename Vec>
    Vector& operator=( const SugarVectorExpression<RTYPE, NA, Vec>& expr ){
        int n = expr.size() ;
        if( n != size() ){
            reset(n) ;    
        }
        expr.apply(*this) ;
        return *this ;
    }
    
private:
    
    inline void reset(int n){
        Storage::set__(Rf_allocVector(RTYPE, n) ) ;        
    }
    
public:
    static inline stored_type get_na() { return traits::get_na<RTYPE>(); }
    static inline bool is_na( stored_type x){ return traits::is_na<RTYPE>(x); }

    inline R_len_t length() const { return ::Rf_length( Storage::get__() ) ; }
    inline R_len_t size() const { return length() ; }

    R_len_t offset(const std::string& name) const {
        SEXP names = RCPP_GET_NAMES( Storage::get__() ) ;
        if( names == R_NilValue ) throw index_out_of_bounds();
        int n = size() ;

        SEXP* data = internal::r_vector_start<STRSXP>(names) ;
        int index = std::find( data, data+n, Rf_mkChar(name.c_str()) ) - data ;
        if( index == n ) throw index_out_of_bounds() ;
        return index ;
    }

    template <typename U>
    void fill( const U& u){
        std::fill( begin(), end(), converter_type::get( u ) ) ;
    }
    
    inline iterator begin() { return get_iterator(0) ; }
    inline iterator end() { return get_iterator(size()) ; }
    
    inline const_iterator begin() const{ return get_const_iterator(0) ; }
    inline const_iterator end() const{ return get_const_iterator(size()) ; }
    
    inline Proxy operator[](int i){ return get_Proxy(i) ;}
    inline const_Proxy operator[](int i) const { return get_constProxy(i); }
    
    inline NameProxy operator[]( const std::string& name ){
        return NameProxy( *this, name ) ;
    }
    inline const_NameProxy operator[]( const std::string& name ) const {
        return const_NameProxy( *this, name ) ;
    }

public:
    
    template <typename... Args> static Vector create(Args... args) {
        return typename create_type<RTYPE, Args...>::type( args... ) ;    
    }

private:
    
    inline stored_type* data(){
        return reinterpret_cast<stored_type*>( DATAPTR(Storage::get__()) );    
    }
    inline const stored_type* data() const{
        return reinterpret_cast<const stored_type*>( DATAPTR(Storage::get__()) );    
    }
    
    // dispatch between simple vectors and proxy based vectors
    inline Proxy get_Proxy(int i){
        return get_Proxy__impl( i, typename std::is_reference<Proxy>::type() ) ;    
    }
    inline Proxy get_Proxy__impl( int i, std::true_type ){
        return *( data() + i ) ;    
    }
    inline Proxy get_Proxy__impl( int i, std::false_type ){
        return Proxy( *this, i ) ;    
    }
    
    inline const_Proxy get_constProxy(int i) const {
        return get_constProxy__impl( i, typename std::is_reference<const_Proxy>::type() ) ;    
    }
    inline const_Proxy get_constProxy__impl( int i, std::true_type ) const {
        return *( data() + i ) ;    
    }
    inline const_Proxy get_constProxy__impl( int i, std::false_type ) const {
        return const_Proxy( *this, i ) ;    
    }
    
    inline iterator get_iterator(int i){
        return get_iterator__impl(i, typename std::is_pointer<iterator>::type() );
    }
    inline iterator get_iterator__impl(int i, std::true_type ){
        return data() + i ;    
    }
    inline iterator get_iterator__impl(int i, std::false_type ){
        return iterator( Proxy(*this, i) );    
    }
    
    inline const_iterator get_const_iterator(int i) const {
        return get_const_iterator__impl(i, typename std::is_pointer<const_iterator>::type() );
    }
    inline const_iterator get_const_iterator__impl(int i, std::true_type ) const {
        return data() + i ;    
    }
    inline const_iterator get_const_iterator__impl(int i, std::false_type ) const {
        return const_iterator( const_Proxy(*this, i) );    
    }
    
} ; /* Vector */

}
#endif
