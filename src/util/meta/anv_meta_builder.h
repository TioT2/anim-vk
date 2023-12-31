#ifndef ANV_META_BUILDER_H_
#define ANV_META_BUILDER_H_

/* Builer head declaration macro */
#define ANV_BUILDER_HEAD(OBJECT_TYPE, SYSTEM_TYPE) class builder { \
  private: \
  friend class SYSTEM_TYPE;  \
  using self = builder;      \
  SYSTEM_TYPE &System;       \
  builder( SYSTEM_TYPE &System ) : System(System) {  }  \
  public: OBJECT_TYPE * Build( VOID );

/* Builder field declaration macro */
#define ANV_BUILDER_FIELD(TYPE, NAME) self & Set##NAME( TYPE &&Value ) { NAME = std::move(Value); return *this; } TYPE NAME

/* Builder end macro */
#define ANV_BUILDER_END }

/* Builder functions implementation declaration macro */
#define ANV_BUILDER_IMPL(OBJECT_TYPE) OBJECT_TYPE * OBJECT_TYPE::builder::Build( VOID ) { return System.Build(*this); }

#endif // !defined(ANV_META_BUILDER_H_)